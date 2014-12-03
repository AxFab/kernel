/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Initialize Memory address space
 */
#include <kernel/memory.h>
#include <kernel/vfs.h>
#include <kernel/info.h>


// ===========================================================================
/** Initialize memory addressing
 */
void kvma_init ()
{
  return;
}


// ---------------------------------------------------------------------------
/** Initialize a new address space structure with a first user-stack */
int addspace_init(kAddSpace_t* space, int flags)
{
  memset(space, 0, sizeof(space));
  space->first_ = KALLOC(kVma_t);
  space->last_ = space->first_;
  space->first_->limit_ = MMU_USERSP_LIMIT;
  space->first_->base_ = MMU_USERSP_LIMIT - PAGE_SIZE;
  space->first_->flags_ = VMA_READ | VMA_WRITE;
  space->vrtPages_ += PAGE_SIZE;
  return __seterrno(0);
}

// ---------------------------------------------------------------------------
/** Initialize a new address space structure with a first user-stack */
kAddSpace_t* kvma_new (size_t peb_size)
{
  kAddSpace_t* addsp = KALLOC(kAddSpace_t);
  peb_size = ALIGN_UP(peb_size, PAGE_SIZE);
  addsp->first_ = KALLOC(kVma_t);
  addsp->last_ = addsp->first_;
  addsp->first_->limit_ = MMU_USERSP_LIMIT;
  addsp->first_->base_ = MMU_USERSP_LIMIT - peb_size;
  addsp->first_->flags_ = VMA_READ | VMA_WRITE;
  addsp->vrtPages_ += peb_size / PAGE_SIZE;
  return addsp;
}


// ---------------------------------------------------------------------------
/** Initialize a new address space structure from an already existing one */
kAddSpace_t* kvma_clone (kAddSpace_t* addp)
{
  kAddSpace_t* addsp = KALLOC(kAddSpace_t);
  kVma_t* md = addp->first_;

  while (md) {
    if (addsp->last_) {
      addsp->last_->next_ = KALLOC(kVma_t);
      addsp->last_->next_->prev_ = addsp->last_;
      addsp->last_ = addsp->last_->next_;

    } else {
      addsp->first_ = KALLOC(kVma_t);
      addsp->last_ = addsp->first_;
    }

    addsp->last_->base_ = md->base_;
    addsp->last_->limit_ = md->limit_;
    addsp->last_->flags_ = md->flags_;
    if (md->ino_) {
      inode_open (md->ino_);
      addsp->last_->ino_ = md->ino_;
      addsp->last_->offset_ = md->offset_;
    }

    md = md->next_;
  }

  addsp->vrtPages_ = addp->vrtPages_;
  return addsp;
}


// ---------------------------------------------------------------------------
/** Free all memory area inforation */
int kvma_destroy (kAddSpace_t* addp)
{
  kVma_t* nx = addp->first_;
  kVma_t* md = addp->first_;

  while (md) {
    nx = md->next_;

    if (md->ino_)
      inode_close (md->ino_);

    kfree(md);
    md = nx;
  }

  kfree(addp);
  return __noerror();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
