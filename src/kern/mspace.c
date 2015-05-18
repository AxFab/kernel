/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Virtual memory area managment.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/map.h>

/* ----------------------------------------------------------------------- */
/** */
static kMemArea_t *area_map_begin_ (kMemSpace_t *sp, kMemArea_t *area)
{
  size_t length;
  int maxLoop = MAX_LOOP;
  kMemArea_t *vma = NULL;
  kMemArea_t *origin = sp->first_;
  size_t base = sp->base_;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  length = (size_t)(area->limit_ - area->address_);

  while (--maxLoop) {
    if (origin->address_ - base >= length) {
      // INSERT BEFORE ORIGIN
      vma = KALLOC(kMemArea_t);
      memcpy (vma, area, sizeof(kMemArea_t));
      vma->next_ = origin;
      vma->prev_ = origin->prev_;

      if (origin->prev_)
        origin->prev_->next_ = vma;

      else
        sp->first_ = vma;

      origin->prev_ = vma;
      vma->address_ = base;
      vma->limit_ = base + length;
      return vma;
    }

    if (origin->next_ != NULL) {
      base = origin->limit_;
      origin = origin->next_;
      continue;
    }

    if (origin->limit_ + length > sp->limit_) {
      return NULL;
    }

    // INSERT LAST
    base = origin->limit_;
    vma = sp->last_ = KALLOC(kMemArea_t);
    memcpy (vma, area, sizeof(kMemArea_t));
    vma = origin->next_ = sp->last_;
    vma->prev_ = origin;
    vma->address_ = base;
    vma->limit_ = base + length;
    return vma;
  }

  __seterrno (ELOOP);
  return NULL;
}


/* ----------------------------------------------------------------------- */
static kMemArea_t* area_first(kMemSpace_t* sp, kMemArea_t* area)
{
  kMemArea_t *vma;

  assert(area->address_ >= sp->base_);
  assert(area->limit_ <= sp->limit_);

  vma = KALLOC(kMemArea_t);
  memcpy (vma, area, sizeof(kMemArea_t));
  vma->next_ = NULL;
  vma->prev_ = NULL;
  sp->first_ = vma;
  sp->last_ = vma;
  return vma;
}

// ---------------------------------------------------------------------------
/** */
static kMemArea_t *area_map_at_ (kMemSpace_t *sp, kMemArea_t *area)
{
  size_t length;
  int maxLoop = MAX_LOOP;
  kMemArea_t *vma = NULL;
  kMemArea_t *origin = sp->first_;
  uintptr_t address = area->address_;
  area->limit_ = ALIGN_UP(area->limit_, PAGE_SIZE);
  length = (size_t)(area->limit_ - area->address_);

  if (!origin) {
    return area_first(sp, area);
  }

  // Try to put as the first memory area
  if (origin->address_ >= address + length) {
    vma = KALLOC(kMemArea_t);
    memcpy (vma, area, sizeof(kMemArea_t));
    vma->next_ = origin;
    vma->prev_ = NULL;
    origin->prev_ = vma;
    sp->first_ = vma;
    return vma;
  }


  while (--maxLoop) {
    if (origin->limit_ > address)  {
      return area_map_begin_(sp, area);
    }

    if (origin->next_ != NULL && origin->next_->address_ >= address + length) {
      vma = KALLOC(kMemArea_t);
      memcpy (vma, area, sizeof(kMemArea_t));
      vma->next_ = origin->next_;
      origin->next_ = vma;
      vma->next_->prev_ = vma;
      vma->prev_ = origin;
      return vma;
      // Put memory area at the end

    } else if (origin->next_ == NULL) {
      vma = KALLOC(kMemArea_t);
      memcpy (vma, area, sizeof(kMemArea_t));
      vma->next_ = NULL;
      sp->last_ = vma;
      origin->next_ = vma;
      vma->prev_ = origin;
      return vma;
    }

    origin = origin->next_;
  }

  __seterrno (ELOOP);
  return NULL;
}


/* ----------------------------------------------------------------------- */
kMemArea_t* area_get(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length)
{
  size_t max;
  kMemArea_t *origin = sp->first_;
  int maxLoop = MAX_LOOP;

  while (origin && --maxLoop) {
    if (origin->ino_ == ino && origin->offset_ <= offset) {

      max = origin->limit_ - origin->address_ + origin->offset_;
      if (offset + length < max)
        return origin;
    }

    origin = origin->next_;
  }

  return NULL;
}


/* ----------------------------------------------------------------------- */
/** Find the area holding an address */
kMemArea_t *area_find(kMemSpace_t* sp, size_t address)
{
  /// @todo DEBUG !
  kMemArea_t *area;
  kMemArea_t *origin;
  int maxLoop = MAX_LOOP;

  klock(&sp->lock_);
  origin = sp->first_;
  while (origin && --maxLoop) {
    if (origin->limit_ > address) {

      if (origin->address_ <= address)
        break;

      kunlock(&sp->lock_);
      return NULL;
    }

    origin = origin->next_;
  }

  area = bb_search_le(&sp->bbTree_, address, kMemArea_t, bbNode_);
  if (area == NULL || area->limit_ < address)
    assert (origin == NULL);
  else
    assert (area == origin);

  kunlock(&sp->lock_);
  return origin;
}


/* ----------------------------------------------------------------------- */
/** Will allocate a new segment on the address space */
kMemArea_t* area_map(kMemSpace_t* sp, size_t length, int flags)
{
  kMemArea_t area = {0};
  kMemArea_t *narea;

  assert (kislocked (&sp->lock_));
  length = ALIGN_UP(length, PAGE_SIZE);
  area.flags_ = flags;
  area.limit_ = length;
  if (length == 0 || !POW2(flags & VMA_TYPE)) {
    __seterrno(EINVAL);
    return NULL;
  }

  if (!sp->first_) {
    area.address_ = sp->base_;
    area.limit_ += sp->base_;
    narea = area_first(sp, &area);

  } else {
    switch (flags & VMA_TYPE) {
    case VMA_SHM:
    case VMA_FILE:
    case VMA_HEAP:
    case VMA_FIFO:
    case VMA_STACK:
      narea = area_map_begin_(sp, &area);
      break;
    default:
      assert (0);
    }
  }

  if (narea == NULL) {
    return NULL;
  }

  narea->flags_ = flags;
  sp->vrtPages_ += length / PAGE_SIZE;
  narea->bbNode_.value_ = (long)narea->address_;
  bb_insert (&sp->bbTree_, &narea->bbNode_);
  return narea;
}


/* ----------------------------------------------------------------------- */
/** Will allocate a new segment at a fixed address on the address space */
kMemArea_t *area_map_at (kMemSpace_t* sp, size_t address, size_t length, int flags)
{
  kMemArea_t *narea;
  kMemArea_t area = {0};

  assert ((flags & VMA_TYPE) != VMA_HEAP);
  assert ((flags & VMA_TYPE) != VMA_STACK);
  assert (kislocked(&sp->lock_));

  area.flags_ = flags;
  area.address_ = address;
  area.limit_ = address + ALIGN_UP(length, PAGE_SIZE);

  if (area.address_ >= area.limit_ || !POW2(flags & VMA_TYPE)) {
    __seterrno(EINVAL);
    return NULL;
  }

  narea = area_map_at_(sp, &area);

  if (narea == NULL) {
    return NULL;
  }

  narea->flags_ = flags;
  sp->vrtPages_ += length / PAGE_SIZE;
  narea->bbNode_.value_ = (long)narea->address_;
  bb_insert (&sp->bbTree_, &narea->bbNode_);
  return narea;
}


/* ----------------------------------------------------------------------- */
int area_attach(kMemArea_t* area, kInode_t* ino, size_t offset)
{
  ///@todo think about link by bucket and add a dir to filter...
  if ((area->flags_ & (VMA_ASSEMBLY | VMA_FILE)) == 0)
    return __seterrno(EINVAL);

  if (inode_open (ino)) {
    return __geterrno();
  }

  area->offset_ = offset;
  area->ino_ = ino;
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/*int area_grow (kMemSpace_t* sp, kMemArea_t *area, size_t extra_size)
{
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if ((area->flags_ & VMA_GROWSUP) != 0) {
    klock (&sp->lock_);

    if (area->next_->address_ >= area->limit_ + extra_size) {
      area->limit_ += extra_size;
      sp->vrtPages_ += extra_size / PAGE_SIZE;
      kunlock (&sp->lock_);
      return __seterrno(0);
    }

    kunlock (&sp->lock_);
    __seterrno (ENOMEM);
  }


  if ((area->flags_ & VMA_GROWSDOWN) != 0) {
    klock (&sp->lock_);

    if (area->prev_->limit_ <= area->address_ - extra_size) {
      area->address_ -= extra_size;
      sp->vrtPages_ += extra_size / PAGE_SIZE;
      kunlock (&sp->lock_);
      return __seterrno(0);
    }

    kunlock (&sp->lock_);
    __seterrno (ENOMEM);
  }

  return __seterrno (EPERM);
}*/


/* ----------------------------------------------------------------------- */
kMemArea_t* area_map_ino(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length, int flags)
{
  kMemArea_t *area;
  klock(&sp->lock_);
  offset = ALIGN_DW(offset, PAGE_SIZE);
  area = area_get(sp, ino, offset, length);
  if (!area) {
    area = area_map(sp, length, VMA_FILE | flags);
    if (area == NULL)
      sched_signal(ENOMEM, length);
    if (area_attach(area, ino, offset))
      kpanic("Try to map a file which is unavailable.\n");
  }

  atomic_inc (&area->usage_);
  kunlock(&sp->lock_);

  if (page_fault(area->address_, PF_KERN)) {
    kpanic ("inode loading failed\n");
  }

  /// @todo what if we are currently reading the device !
  return area;
}


/* ----------------------------------------------------------------------- */
void area_unmap(kMemSpace_t* sp, kMemArea_t* area)
{
  atomic_dec (&area->usage_);
}


/* ----------------------------------------------------------------------- */
/** Initialize a new address space structure with a first user-stack */
int area_init(kMemSpace_t* sp, size_t base, size_t length)
{
  memset(sp, 0, sizeof(kMemSpace_t));
  sp->base_ = base;
  sp->base_ = ALIGN_UP(sp->base_, PAGE_SIZE);
  length -= sp->base_ - base;
  length = ALIGN_DW(length, PAGE_SIZE);
  sp->limit_ =  length + sp->base_;
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** Will map an assembly on the address space */
static kMemArea_t *area_map_section (kMemSpace_t *sp, kSection_t *section, kInode_t *ino)
{
  kMemArea_t * area;
  int flags = (section->flags_ & (VMA_ACCESS | VMA_ASSEMBLY)) | VMA_FILE;

  if ((flags & VMA_WRITE) == 0)
    flags |= VMA_SHARED;

  area = area_map_at (sp, section->address_, section->length_, flags);
  if (area == NULL)
    return NULL;

  // @todo atomic_inc (&area->usage_); -- used for FILE several fd map the same
  if (area_attach (area, ino, section->offset_)) {
    // area_unmap_area (mspace, area);
    return NULL;
  }

  return area;
}


/* ----------------------------------------------------------------------- */
int area_assembly (kMemSpace_t *sp, kAssembly_t* assembly)
{
  kSection_t *section;
  klock (&sp->lock_);
  ll_for_each(&assembly->sections_, section, kSection_t, node_) {
    area_map_section (sp, section, assembly->ino_);
  }

  kunlock (&sp->lock_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
void scavenge_area(kMemSpace_t* sp)
{
  kMemArea_t *origin;
  int maxLoop = MAX_LOOP;

  klock(&sp->lock_);
  origin = sp->first_;
  while (origin && --maxLoop) {

    if (origin->ino_ != NULL && origin->usage_ == 0) {
      inode_close(origin->ino_);
      origin->ino_ = NULL;
      /// @todo REMOVE FROM MSPACE
    }

    origin = origin->next_;
  }

  kunlock(&sp->lock_);
}


/* ----------------------------------------------------------------------- */
int area_destroy(kMemSpace_t* sp)
{
  kMemArea_t *origin;
  kMemArea_t *sweep;
  int maxLoop = MAX_LOOP;

  klock(&sp->lock_);
  origin = sp->first_;
  while (origin && --maxLoop) {

    if (origin->ino_ != NULL /*&& origin->usage_ == 0*/) {
      inode_close(origin->ino_);
      origin->ino_ = NULL;
    }

    // @todo Released pages
    sweep = origin;
    origin = origin->next_;
    kfree (sweep);
  }

  // assert (sp->phyPages_ == 0);
  mmu_destroy_userspace(sp);
  kunlock(&sp->lock_);
  memset(sp, 0, sizeof(*sp));
  return 0;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
