#include <memory.h>
#include <inodes.h>

// ============================================================================

static kVma_t* kVma_MapAtBegin (kAddSpace_t* addp, kVma_t* area)
{
  kVma_t* vma = NULL;
  kVma_t* origin = addp->first_;
  uintptr_t base = USR_SPACE_BASE;
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

    if (origin->limit_ + length > USR_SPACE_LIMIT) {
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

// ============================================================================

static kVma_t* kVma_MapAt (kAddSpace_t* addp, kVma_t* area)
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
      return kVma_MapAtBegin (addp, area);
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

// ============================================================================

// Try before an address (BEFORE HEAP, BEFORE STACK - to preserve heap)
// static kVma_t* kVma_MapBefore (kAddSpace_t* addp, kVma_t* area)
// {
//   __seterrno(ENOSYS);
//   return NULL;
// }

// ============================================================================

kVma_t* kVma_MMap (kAddSpace_t* addressSpace, kVma_t* area)
{
  kVma_t* vma;

  // TODO move area check here

  if (area->ino_)
    kFs_Open(area->ino_); // TODO if fail !?

  klock (&addressSpace->lock_);

  if (area->base_ != 0)
    vma = kVma_MapAt (addressSpace, area);

  else
    vma = kVma_MapAtBegin (addressSpace, area);

  if (vma == NULL && area->ino_)
    kFs_Close (area->ino_);

  if (vma != NULL)
    addressSpace->vrtPages_ += (vma->limit_ - vma->base_) / PAGE_SIZE;

  kunlock (&addressSpace->lock_);
  return vma;
}

// ============================================================================

int kVma_Unmap (kAddSpace_t* addp, uintptr_t address, size_t length)
{
  return __seterrno(ENOSYS);
}


// ============================================================================

int kVma_GrowUp (kAddSpace_t* addp, void* address, size_t extra_size)
{
  kVma_t* vma = kVma_FindAt (addp, (uintptr_t)address);
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if (!vma)
    return __geterrno();

  if ((vma->flags_ & VMA_GROWSUP) == 0)
    return __seterrno (EPERM);

  __noerror();
  klock (&addp->lock_);

  if (vma->next_->base_ >= vma->limit_ + extra_size) {
    vma->limit_ += extra_size;
    addp->vrtPages_ += extra_size / PAGE_SIZE;

  } else {
    __seterrno (ENOMEM);
  }

  kunlock (&addp->lock_);
  return __geterrno();
}

// ============================================================================

int kVma_GrowDown (kAddSpace_t* addp, void* address, size_t extra_size)
{
  kVma_t* vma = kVma_FindAt (addp, (uintptr_t)address);
  extra_size = ALIGN_UP(extra_size, PAGE_SIZE);

  if (!vma)
    return __geterrno();

  if ((vma->flags_ & VMA_GROWSDOWN) == 0)
    return __seterrno (EPERM);

  __noerror();
  klock (&addp->lock_);

  if (vma->prev_->limit_ <= vma->base_ - extra_size) {
    vma->base_ -= extra_size;
    addp->vrtPages_ += extra_size / PAGE_SIZE;

  } else {
    __seterrno (ENOMEM);
  }

  kunlock (&addp->lock_);
  return __geterrno();
}


