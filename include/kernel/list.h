#ifndef KERNEL_LIST_H__
#define KERNEL_LIST_H__

#include <kernel/spinlock.h>


static inline void klist_push_back(anchor_t* head, list_t* item)
{
  assert (item->prev_ == NULL);
  assert (item->next_ == NULL);

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
  assert (item->prev_ == NULL);
  assert (item->next_ == NULL);

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

#define klist_pop(h,t,m) (t*)klist_pop_((h), offsetof(t,m))
static inline void* klist_pop_(anchor_t* head, size_t off) 
{
  list_t* item = head->first_;
  if (item == NULL)
    return NULL;

  klock (&head->lock_);
  head->first_ = head->first_->next_;
  if (head->first_)
    head->first_->prev_ = NULL;
  item->prev_ = NULL;
  item->next_ = NULL;
  --head->count_;
  kunlock (&head->lock_);
  return ((char*)item) - off;
}

static inline void klist_remove (anchor_t* head, list_t* item)
{
  klock (&head->lock_);

  list_t* w = item; // @todo try something faster!
  while (w->prev_) w = w->prev_;
  // @test Check that useless loop is optimized or add #if
  assert (w == head->first_);

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

static inline void klist_remove_if (anchor_t* head, list_t* item)
{
  klock (&head->lock_);

  list_t* w = item; // @todo try something faster!
  while (w->prev_) w = w->prev_;

  if (w != head->first_) {
    kunlock (&head->lock_);
    return;
  }

  kunlock (&head->lock_);
  klist_remove (head, item);
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

#define for_each(v,h,s,m) for ((v)=klist_begin(h,s,m);(v);(v)=klist_next(v,s,m))


#endif /* KERNEL_LIST_H__ */
