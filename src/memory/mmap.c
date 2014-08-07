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
#include <kernel/inodes.h>
#include <kernel/info.h>

// ===========================================================================
/** */
static kVma_t* kvma_map_begin (kAddSpace_t* addp, kVma_t* area)
{
  kVma_t* vma = NULL;
  kVma_t* origin = addp->first_;
  uintptr_t base = kHDW.userSpaceBase_;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  size_t length = (size_t)(area->limit_ - area->base_);
  int maxLoop = MAX_LOOP_BUCKET;

  while (--maxLoop) {
    if (origin->base_ - base >= length) {
      // INSERT BEFORE ORIGIN
      vma = (kVma_t*)kalloc(sizeof(kVma_t));
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = origin;
      vma->prev_ = origin->prev_;

      if (origin->prev_)
        origin->prev_->next_ = vma;

      else
        addp->first_ = vma;

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

    if (origin->limit_ + length > kHDW.userSpaceLimit_) {
      return NULL;
    }

    // INSERT LAST
    base = origin->limit_;
    addp->last_ = (kVma_t*)kalloc(sizeof(kVma_t));
    memcpy (vma, area, sizeof(kVma_t));
    vma = origin->next_ = addp->last_;
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
static kVma_t* kvma_map_at (kAddSpace_t* addp, kVma_t* area)
{
  kVma_t* vma = NULL;
  kVma_t* origin = addp->first_;
  uintptr_t address = area->base_;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  size_t length = (size_t)(area->limit_ - area->base_);

  // Try to put as the first memory area
  if (origin->base_ >= address + length) {
    vma = (kVma_t*)kalloc(sizeof(kVma_t));
    memcpy (vma, area, sizeof(kVma_t));
    vma->next_ = origin;
    vma->prev_ = NULL;
    origin->prev_ = vma;
    addp->first_ = vma;
    return vma;
  }

  int maxLoop = MAX_LOOP_BUCKET;

  while (--maxLoop) {
    if (origin->limit_ > address)  {
      return kvma_map_begin (addp, area);
    }

    if (origin->next_ != NULL && origin->next_->base_ >= address + length) {
      vma = (kVma_t*)kalloc(sizeof(kVma_t));
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = origin->next_;
      origin->next_ = vma;
      vma->next_->prev_ = vma;
      vma->prev_ = origin;
      return vma;
      // Put memory area at the end

    } else if (origin->next_ == NULL) {
      vma = (kVma_t*)kalloc(sizeof(kVma_t));
      memcpy (vma, area, sizeof(kVma_t));
      vma->next_ = NULL;
      addp->last_ = vma;
      origin->next_ = vma;
      vma->prev_ = origin;
      return vma;
    }

    origin = origin->next_;
  }

  __seterrno (ELOOP);
  return NULL;
}


// ---------------------------------------------------------------------------
// Try before an address (BEFORE HEAP, BEFORE STACK - to preserve heap)
// static kVma_t* kvma_map_before (kAddSpace_t* addp, kVma_t* area)
// {
//   __seterrno(ENOSYS);
//   return NULL;
// }


// ===========================================================================
/** */
kVma_t* kvma_mmap (kAddSpace_t* addressSpace, kVma_t* area)
{
  kVma_t* vma;

  // TODO move area check here

  if (area->ino_)
    kfs_grab(area->ino_); // TODO if fail !?

  klock (&addressSpace->lock_, LOCK_VMA_MMAP);

  if (area->base_ != 0)
    vma = kvma_map_at (addressSpace, area);

  else
    vma = kvma_map_begin (addressSpace, area);

  if (vma == NULL && area->ino_)
    kfs_release (area->ino_);

  if (vma != NULL) {
    addressSpace->vrtPages_ += (vma->limit_ - vma->base_) / PAGE_SIZE;

    vma->flags_ = area->flags_;
    vma->offset_ = area->offset_;
    vma->ino_ = area->ino_;

    // assert ((vma->offset_ & (PAGE_SIZE-1)) == 0);
  }

  kunlock (&addressSpace->lock_);
  return vma;
}


// ---------------------------------------------------------------------------

int kvma_unmap (kAddSpace_t* addp, uintptr_t address, size_t length)
{
  return __seterrno(ENOSYS);
}


// ---------------------------------------------------------------------------

int kvma_grow_up (kAddSpace_t* addp, void* address, size_t extra_size)
{
  kVma_t* vma = kvma_look_at (addp, (uintptr_t)address);
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if (!vma)
    return __geterrno();

  if ((vma->flags_ & VMA_GROWSUP) == 0)
    return __seterrno (EPERM);

  __noerror();
  klock (&addp->lock_, LOCK_VMA_GROW);

  if (vma->next_->base_ >= vma->limit_ + extra_size) {
    vma->limit_ += extra_size;
    addp->vrtPages_ += extra_size / PAGE_SIZE;

  } else {
    __seterrno (ENOMEM);
  }

  kunlock (&addp->lock_);
  return __geterrno();
}


// ---------------------------------------------------------------------------

int kvma_grow_down (kAddSpace_t* addp, void* address, size_t extra_size)
{
  kVma_t* vma = kvma_look_at (addp, (uintptr_t)address);
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if (!vma)
    return __geterrno();

  if ((vma->flags_ & VMA_GROWSDOWN) == 0)
    return __seterrno (EPERM);

  __noerror();
  klock (&addp->lock_, LOCK_VMA_GROW);

  if (vma->prev_->limit_ <= vma->base_ - extra_size) {
    vma->base_ -= extra_size;
    addp->vrtPages_ += extra_size / PAGE_SIZE;

  } else {
    __seterrno (ENOMEM);
  }

  kunlock (&addp->lock_);
  return __geterrno();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
