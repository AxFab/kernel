#ifndef KERNEL_LIST_H__
#define KERNEL_LIST_H__

#include <kernel/spinlock.h>

typedef struct list         list_t;
typedef struct anchor       anchor_t;
struct anchor
{
  spinlock_t  lock_;
  list_t*     first_;
  list_t*     last_;
  int         count_;
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
  ++head->count_;
  kunlock (&head->lock_);
}

static inline void klist_push_front(anchor_t* head, list_t* item)
{
  klock (&head->lock_);
  item->next_ = head->first_;
  if (head->first_ != NULL)
    head->first_->prev_ = item;
  else 
    head->last_ = item;
  item->prev_ = NULL;
  head->first_ = item;
  ++head->count_;
  kunlock (&head->lock_);
}

static inline void klist_remove (anchor_t* head, list_t* item)
{
  klock (&head->lock_);

  if (item->prev_) {
    item->prev_->next_ = item->next_;
  } else {
    assert (head->first_ == item);
    head->first_ = item->next_;
  }

  if (item->next_) {
    item->next_->prev_ = item->prev_;
  } else {
    assert (head->last_ == item);
    head->last_ = item->prev_;
  }

  item->prev_ = NULL;
  item->next_ = NULL;
  --head->count_;
  kunlock (&head->lock_);
}

static inline int list_isdetached (list_t* item) 
{
  return item->prev_ == NULL && item->next_ == NULL;
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
