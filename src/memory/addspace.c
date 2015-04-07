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


// ---------------------------------------------------------------------------
/** Initialize a new address space structure with a first user-stack */
int addspace_init(kAddSpace_t *mspace, int flags)
{
  memset(mspace, 0, sizeof(mspace));
  mspace->first_ = KALLOC(kVma_t);
  mspace->last_ = mspace->first_;
  mspace->first_->limit_ = MMU_USERSP_LIMIT;
  mspace->first_->base_ = MMU_USERSP_LIMIT - PAGE_SIZE;
  mspace->first_->flags_ = VMA_READ | VMA_WRITE;
  mspace->vrtPages_ += PAGE_SIZE;
  mspace->first_->bbNode_.value_ = (long)(MMU_USERSP_LIMIT - PAGE_SIZE);
  return __seterrno(0);
}


// ---------------------------------------------------------------------------
/** Find the area holding an address */
kVma_t *addspace_find(kAddSpace_t *mspace, size_t address)
{
  kVma_t *origin = mspace->first_;
  int maxLoop = MAX_LOOP_BUCKET;

  while (origin && --maxLoop) {
    if (origin->limit_ > address) {

      if (origin->base_ <= address)
        break;

      return NULL;
    }

    origin = origin->next_;
  }

  kVma_t *area = get_item(aa_search_lesseq(&mspace->bbTree_, (long)address), kVma_t, bbNode_);
  // kprintf ("addspace_find <%x - %x>\n", area, origin);
  // if (area != NULL)
  //   kprintf ("vma 1 <%8x-%8x  %5x >\n", area->base_, area->limit_, area->flags_, *((char**)area->ino_));
  // else
  //   kprintf ("vma 1 <null>\n");

  // if (origin != NULL)
  //   kprintf ("vma 2 <%8x-%8x  %5x  %s>\n", origin->base_, origin->limit_, origin->flags_, *((char**)origin->ino_));
  // else
  //   kprintf ("vma 2 <null>\n");
  if (area == NULL || area->limit_ < address)
    assert (origin == NULL);
  else
    assert (area == origin);

  return origin;
}


// ---------------------------------------------------------------------------

void addspace_display (kAddSpace_t *mspace)
{
  const char *rights[] = {
    "----", "---x", "--w-",  "--wx",
    "-r--", "-r-x", "-rw-",  "-rwx",
    "S---", "S--x", "S-w-",  "S-wx",
    "Sr--", "Sr-x", "Srw-",  "Srwx"
  };
  int i = 0;
  void *ptr = NULL;
  kVma_t *bk = mspace->first_;
  kprintf ("MMap debug display ----- %d <%s>\n",
           mspace->vrtPages_, kpsize(mspace->vrtPages_ * PAGE_SIZE));

  long pages = 0;

  while (bk != NULL) {
    assert (bk->prev_ == ptr);

    // assert ((bk->limit_ - bk->base_) == bk->length_);
    pages += (bk->limit_ - bk->base_) / PAGE_SIZE;
    kprintf ("BB[%3d] - ", bk->bbNode_.level_);

    if (bk->ino_)
      kprintf ("%2d] [0x%16x - 0x%16x] %s - <%s>  %s :0x%x\n", ++i,
               (uint32_t)bk->base_, (uint32_t)bk->limit_, rights[bk->flags_ & 0xf],
               kpsize(bk->limit_ - bk->base_),  (*(char **)bk->ino_), (size_t)bk->offset_);

    else
      kprintf ("%2d] [0x%16x - 0x%16x] %s - <%s>  %s\n", ++i,
               (uint32_t)bk->base_, (uint32_t)bk->limit_, rights[bk->flags_ & 0xf],
               kpsize(bk->limit_ - bk->base_),
               (bk->flags_ & VMA_STACK ? "[stack]" : (bk->flags_ & VMA_HEAP ? "[heap]" : "---")));

    assert (bk->base_ == (size_t)bk->bbNode_.value_);

    ptr = bk;
    bk = bk->next_;
  }

  // TODO assert (mspace->vrtPages_ == );
}

// // ---------------------------------------------------------------------------
// /** Initialize a new address space structure with a first user-stack */
// kAddSpace_t* kvma_new (size_t peb_size)
// {
//   kAddSpace_t* addsp = KALLOC(kAddSpace_t);
//   peb_size = ALIGN_UP(peb_size, PAGE_SIZE);
//   addsp->first_ = KALLOC(kVma_t);
//   addsp->last_ = addsp->first_;
//   addsp->first_->limit_ = MMU_USERSP_LIMIT;
//   addsp->first_->base_ = MMU_USERSP_LIMIT - peb_size;
//   addsp->first_->flags_ = VMA_READ | VMA_WRITE;
//   addsp->vrtPages_ += peb_size / PAGE_SIZE;
//   return addsp;
// }


// // ---------------------------------------------------------------------------
// /** Initialize a new address space structure from an already existing one */
// kAddSpace_t* kvma_clone (kAddSpace_t* addp)
// {
//   kAddSpace_t* addsp = KALLOC(kAddSpace_t);
//   kVma_t* md = addp->first_;

//   while (md) {
//     if (addsp->last_) {
//       addsp->last_->next_ = KALLOC(kVma_t);
//       addsp->last_->next_->prev_ = addsp->last_;
//       addsp->last_ = addsp->last_->next_;

//     } else {
//       addsp->first_ = KALLOC(kVma_t);
//       addsp->last_ = addsp->first_;
//     }

//     addsp->last_->base_ = md->base_;
//     addsp->last_->limit_ = md->limit_;
//     addsp->last_->flags_ = md->flags_;
//     if (md->ino_) {
//       inode_open (md->ino_);
//       addsp->last_->ino_ = md->ino_;
//       addsp->last_->offset_ = md->offset_;
//     }

//     md = md->next_;
//   }

//   addsp->vrtPages_ = addp->vrtPages_;
//   return addsp;
// }


// // ---------------------------------------------------------------------------
// /** Free all memory area inforation */
// int kvma_destroy (kAddSpace_t* addp)
// {
//   kVma_t* nx = addp->first_;
//   kVma_t* md = addp->first_;

//   while (md) {
//     nx = md->next_;

//     if (md->ino_)
//       inode_close (md->ino_);

//     kfree(md);
//     md = nx;
//   }

//   kfree(addp);
//   return __noerror();
// }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
