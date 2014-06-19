#include <memory.h>
#include <inodes.h>


int kVma_Initialize ()
{
  return __noerror();
}


kAddSpace_t* kVma_New (size_t stack_size)
{
  kAddSpace_t* addsp = (kAddSpace_t*)kalloc(sizeof(kAddSpace_t));
  stack_size = ALIGN_UP(stack_size, PAGE_SIZE);
  addsp->first_ = (kVma_t*)kalloc(sizeof(kVma_t));
  addsp->last_ = addsp->first_;
  addsp->first_->limit_ = USR_SPACE_LIMIT;
  addsp->first_->base_ = USR_SPACE_LIMIT - stack_size;
  addsp->first_->flags_ = VMA_STACK | VMA_GROWSDOWN | VMA_READ | VMA_WRITE;
  addsp->vrtPages_ += stack_size / PAGE_SIZE;
  return addsp;
}


kAddSpace_t* kVma_Clone (kAddSpace_t* addp)
{
  kAddSpace_t* addsp = (kAddSpace_t*)kalloc(sizeof(kAddSpace_t));
  kVma_t* md = addp->first_;

  while (md) {
    if (addsp->last_) {
      addsp->last_->next_ = (kVma_t*)kalloc(sizeof(kVma_t));
      addsp->last_ = addsp->last_->next_;

    } else {
      addsp->first_ = (kVma_t*)kalloc(sizeof(kVma_t));
      addsp->last_ = addsp->first_;
    }

    memcpy (addsp->last_, md, sizeof(kVma_t));
    md = md->next_;
  }

  return addsp;
}

int kVma_Destroy (kAddSpace_t* addp)
{
  kVma_t* nx = addp->first_;
  kVma_t* md = addp->first_;

  while (md) {
    nx = md->next_;

    if (md->ino_)
      kFs_Close (md->ino_);

    kfree(md);
    md = nx;
  }

  kfree(addp);
  return __noerror();
}