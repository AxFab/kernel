#ifndef KERNEL_LIST_H__
#define KERNEL_LIST_H__

#include <kernel/core.h>

typedef struct list         list_t;
typedef struct anchor       anchor_t;
struct anchor
{
  spinlock_t  lock_;
  list_t*     first_;
  list_t*     last_;
};

struct list 
{
  list_t*     prev_;
  list_t*     next_;
};

#define ANCHOR_INIT  {LOCK_INIT, NULL, NULL}

static inline void klist_push_back(anchor_t* head, list_t* item)
{
  klock (&head->lock_);
  item->prev_ = head->last_;
  if (head->last_ != NULL)
    head->last_->next_ = item;
  else 
    head->first_ = item;
  item->next_ = NULL;
  head->last_ = item;
  kunlock (&head->lock_);
}

#define klist_begin(h,t,m) (t*)klist_begin_((h), offsetof(t,m))
static inline void* klist_begin_(anchor_t* head, size_t off) 
{
  if (head->first_ == NULL)
    return  NULL;
  return ((char*)head->first_) - off;
}

#define klist_next(i,t,m) (t*)klist_next_((i), offsetof(t,m))
static inline void* klist_next_(void* item, size_t off) 
{
  list_t* node = (list_t*)(((char*)item) + off);
  if (node->next_ == NULL)
    return NULL;
  return ((char*)node->next_) - off;
}



#endif /* KERNEL_LIST_H__ */
