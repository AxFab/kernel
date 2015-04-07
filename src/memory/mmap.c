/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Memory mapping routines
 */
#include <kernel/memory.h>
#include <kernel/vfs.h>
#include <kernel/info.h>
#include <kernel/task.h>


/** */
static kVma_t *kvma_map_begin (kAddSpace_t *mspace, kVma_t *area)
{
  kVma_t *vma = NULL;
  kVma_t *origin = mspace->first_;
  uintptr_t base = MMU_USERSP_BASE;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  size_t length = (size_t)(area->limit_ - area->base_);
  int maxLoop = MAX_LOOP_BUCKET;

  while (--maxLoop) {
    if (origin->base_ - base >= length) {
      // INSERT BEFORE ORIGIN
      vma = KALLOC(kVma_t);
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = origin;
      vma->prev_ = origin->prev_;

      if (origin->prev_)
        origin->prev_->next_ = vma;

      else
        mspace->first_ = vma;

      origin->prev_ = vma;
      vma->base_ = base;
      vma->limit_ = base + length;
      return vma;
    }

    if (origin->next_ != NULL) {
      base = origin->limit_;
      origin = origin->next_;
      continue;
    }

    if (origin->limit_ + length > MMU_USERSP_LIMIT) {
      return NULL;
    }

    // INSERT LAST
    base = origin->limit_;
    mspace->last_ = KALLOC(kVma_t);
    memcpy (vma, area, sizeof(kVma_t));
    vma = origin->next_ = mspace->last_;
    vma->prev_ = origin;
    vma->base_ = base;
    vma->limit_ = base + length;
    return vma;
  }

  __seterrno (ELOOP);
  return NULL;
}


// ---------------------------------------------------------------------------
/** */
static kVma_t *kvma_map_at (kAddSpace_t *mspace, kVma_t *area)
{
  kVma_t *vma = NULL;
  kVma_t *origin = mspace->first_;
  uintptr_t address = area->base_;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  size_t length = (size_t)(area->limit_ - area->base_);

  // Try to put as the first memory area
  if (origin->base_ >= address + length) {
    vma = KALLOC(kVma_t);
    memcpy (vma, area, sizeof(kVma_t));
    vma->next_ = origin;
    vma->prev_ = NULL;
    origin->prev_ = vma;
    mspace->first_ = vma;
    return vma;
  }

  int maxLoop = MAX_LOOP_BUCKET;

  while (--maxLoop) {
    if (origin->limit_ > address)  {
      return kvma_map_begin (mspace, area);
    }

    if (origin->next_ != NULL && origin->next_->base_ >= address + length) {
      vma = KALLOC(kVma_t);
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = origin->next_;
      origin->next_ = vma;
      vma->next_->prev_ = vma;
      vma->prev_ = origin;
      return vma;
      // Put memory area at the end

    } else if (origin->next_ == NULL) {
      vma = KALLOC(kVma_t);
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = NULL;
      mspace->last_ = vma;
      origin->next_ = vma;
      vma->prev_ = origin;
      return vma;
    }

    origin = origin->next_;
  }

  __seterrno (ELOOP);
  return NULL;
}





// ===========================================================================
//      Map area
// ===========================================================================

// ---------------------------------------------------------------------------
/** Will allocate a new segment on the address space */
kVma_t *vmarea_map (kAddSpace_t *mspace, size_t length, int flags)
{
  kVma_t area = {0};
  area.flags_ = flags;
  area.limit_ = length;

  if (length == 0 || !IS_PW2(flags & VMA_TYPE)) {
    __seterrno(EINVAL);
    return NULL;
  }

  klock (&mspace->lock_);
  kVma_t *narea;

  switch (flags & VMA_TYPE) {
  case VMA_SHM:
  case VMA_FILE:
  case VMA_HEAP:
  case VMA_STACK:
    narea = kvma_map_begin (mspace, &area);
    break;
  default:
    assert (0);
  }

  if (narea == NULL) {
    kunlock (&mspace->lock_);
    return NULL;
  }

  narea->flags_ = flags;
  mspace->vrtPages_ += length / PAGE_SIZE;
  narea->bbNode_.value_ = (long)narea->base_;
  aa_insert (&mspace->bbTree_, &narea->bbNode_);
  kunlock (&mspace->lock_);
  return narea;
}


// ---------------------------------------------------------------------------
/** Will allocate a new segment at a fixed address on the address space */
kVma_t *vmarea_map_at (kAddSpace_t *mspace, size_t address, size_t length, int flags)
{
  assert ((flags & VMA_TYPE) != VMA_HEAP);
  assert ((flags & VMA_TYPE) != VMA_STACK);

  kVma_t area = {0};
  area.flags_ = flags;
  area.base_ = address;
  area.limit_ = address + length;

  if (area.base_ >= area.limit_ || !IS_PW2(flags & VMA_TYPE)) {
    __seterrno(EINVAL);
    return NULL;
  }

  klock (&mspace->lock_);
  kVma_t *narea = kvma_map_at (mspace, &area);

  if (narea == NULL) {
    kunlock (&mspace->lock_);
    return NULL;
  }

  narea->flags_ = flags;
  mspace->vrtPages_ += length / PAGE_SIZE;
  narea->bbNode_.value_ = (long)narea->base_;
  aa_insert (&mspace->bbTree_, &narea->bbNode_);
  kunlock (&mspace->lock_);
  return narea;
}


// ===========================================================================
//      Map helper
// ===========================================================================

// ---------------------------------------------------------------------------
/** Will map an assembly on the address space */
kVma_t *vmarea_map_section (kAddSpace_t *mspace, kSection_t *section, kInode_t *ino)
{
  int flags = (section->flags_ & (VMA_ACCESS | VMA_ASSEMBLY)) | VMA_FILE;

  if ((flags & VMA_WRITE) == 0)
    flags |= VMA_SHARED;

  kVma_t *area = vmarea_map_at (mspace, section->address_, section->length_, flags);

  if (area == NULL)
    return NULL;

  if (vmarea_map_ino (area, ino, section->offset_)) {
    vmarea_unmap_area (mspace, area);
    return NULL;
  }

  return area;
}


// ---------------------------------------------------------------------------
int vmarea_map_ino (kVma_t *area, kInode_t *ino, size_t offset)
{
  //@todo think about link by bucket and add a dir to filter...
  if ((area->flags_ & (VMA_ASSEMBLY | VMA_FILE)) == 0)
    return __seterrno(EINVAL);

  if (inode_open (ino)) {
    return __geterrno();
  }

  area->offset_ = offset;
  area->ino_ = ino;
  return __seterrno(0);
}

// ---------------------------------------------------------------------------
int vmarea_grow (kAddSpace_t *mspace, kVma_t *area, size_t extra_size)
{
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if ((area->flags_ & VMA_GROWSUP) != 0) {
    klock (&mspace->lock_);

    if (area->next_->base_ >= area->limit_ + extra_size) {
      area->limit_ += extra_size;
      mspace->vrtPages_ += extra_size / PAGE_SIZE;
      kunlock (&mspace->lock_);
      return __seterrno(0);
    }

    kunlock (&mspace->lock_);
    __seterrno (ENOMEM);
  }


  if ((area->flags_ & VMA_GROWSDOWN) != 0) {
    klock (&mspace->lock_);

    if (area->prev_->limit_ <= area->base_ - extra_size) {
      area->base_ -= extra_size;
      mspace->vrtPages_ += extra_size / PAGE_SIZE;
      kunlock (&mspace->lock_);
      return __seterrno(0);
    }

    kunlock (&mspace->lock_);
    __seterrno (ENOMEM);
  }

  return __seterrno (EPERM);
}

// ===========================================================================
//      Unmap
// ===========================================================================

// ---------------------------------------------------------------------------
void vmarea_unmap_area (kAddSpace_t *mspace, kVma_t *area)
{
}


// ---------------------------------------------------------------------------
void vmarea_unmap (kAddSpace_t *mspace, size_t address, size_t length)
{
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
