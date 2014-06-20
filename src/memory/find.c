#include <memory.h>

// ============================================================================


kVma_t* kVma_FindAt (kAddSpace_t* addp, uintptr_t address)
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

// ----------------------------------------------------------------------------
kVma_t* kVma_FindFile (kAddSpace_t* addp, kInode_t* ino, off_t offset)
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
    kVma_t vma = { 0, 0, filemap, NULL, NULL, ino, ALIGN_DW (offset, filemap) };
    return kVma_MMap (addp, &vma);
  }

  return origin;
}

void kVma_Display(kAddSpace_t* addp)
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


