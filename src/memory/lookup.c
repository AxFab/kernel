/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Reach information about memory area
 */
#include <kernel/memory.h>


// ===========================================================================
/** */
kVma_t* kvma_look_at (kAddSpace_t* addp, uintptr_t address)
{
  kVma_t* origin = addp->first_;
  int maxLoop = MAX_LOOP_BUCKET;

  while (origin && --maxLoop) {
    if (origin->limit_ > address) {

      if (origin->base_ <= address)
        break;

      return NULL;
    }

    origin = origin->next_;
  }

  return origin;
}


// ---------------------------------------------------------------------------
/** */
kVma_t* kvma_look_ino (kAddSpace_t* addp, kInode_t* ino, off_t offset)
{
  kVma_t* origin = addp->first_;
  int maxLoop = MAX_LOOP_BUCKET;

  while (origin && --maxLoop) {
    if (origin->ino_ == ino &&
        (origin->flags_ & VMA_SHARED) &&
        origin->offset_ <= offset &&
        origin->offset_ + (off_t)(origin->limit_ - origin->base_) > offset)
      break;

    origin = origin->next_;
  }

  if (!(origin && --maxLoop)) {
    size_t filemap = FILE_MAP_SIZE;
    // FIXME eventually freed the old ones.
    // FIXME wrong flags
    kVma_t vma = { VMA_SHARED | VMA_READ | VMA_WRITE, 0, filemap, NULL, NULL, ino, ALIGN_DW (offset, filemap) };
    origin = kvma_mmap (addp, &vma);

    // FIXME - Already try to map it...

    size_t add = (size_t)origin->base_;
    for (;add < (size_t)origin->limit_; add += PAGE_SIZE) {
      kprintf ("For inode %s, try to map at %x \n", *((char**)ino), add);
      // kpg_fill_stream (origin, add, PG_USER_RDWR);
    }
  }

  return origin;
}


// ---------------------------------------------------------------------------
/** */
void kvma_display(kAddSpace_t* addp)
{
  const char* rights[] = {
    "----", "---x", "--w-",  "--wx",
    "-r--", "-r-x", "-rw-",  "-rwx",
    "S---", "S--x", "S-w-",  "S-wx",
    "Sr--", "Sr-x", "Srw-",  "Srwx"
  };
  int i = 0;
  void* ptr = NULL;
  kVma_t* bk = addp->first_;
  kprintf ("MMap debug display ----- %d <%s>\n",
           addp->vrtPages_, kpsize(addp->vrtPages_ * PAGE_SIZE));

  while (bk != NULL) {
    assert (bk->prev_ == ptr);
    if (bk->ino_)
      kprintf ("%2d] [0x%16x - 0x%16x] %s - <%s>  %s :0x%x\n", ++i,
               (uint32_t)bk->base_, (uint32_t)bk->limit_, rights[bk->flags_ & 0xf],
               kpsize(bk->limit_ - bk->base_),  (*(char**)bk->ino_), (size_t)bk->offset_);

    else
      kprintf ("%2d] [0x%16x - 0x%16x] %s - <%s>  %s\n", ++i,
               (uint32_t)bk->base_, (uint32_t)bk->limit_, rights[bk->flags_ & 0xf],
               kpsize(bk->limit_ - bk->base_),
               (bk->flags_ & VMA_STACK ? "[stack]" : (bk->flags_ & VMA_HEAP ? "[heap]" : "---")));

    ptr = bk;
    bk = bk->next_;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
