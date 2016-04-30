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
 *      Linked-list implementation.
 */
#ifndef _SKC_LLIST_H 
#define _SKC_LLIST_H  1

#include <cdefs/stddef.h>
#include <assert.h>


#define ll_append(h,n)        ll_push_back(h,n)
#define ll_next(v,t,m)        itemof((v)->m.next_,t,m) 
#define ll_enqueue(h,n)       ll_push_front(h,n)
#define ll_dequeue(h,t,m)     ll_pop_back(h,t,m)
#define ll_push(h,n)          ll_push_back(h,n)
#define ll_pop(h,t,m)         ll_pop_back(h,t,m)
#define ll_peek(h,t,m)        itemof((h)->last_,t,m)
#define ll_pop_back(h,t,m)    itemof(ll_pop_back_(h),t,m)
#define ll_first(h,t,m)       itemof((h)->first_,t,m)
#define ll_foreach(h,v,t,m)  \
  for ((v)=ll_first(h,t,m);(v);(v)=(t*)ll_next(v,t,m))



/* ----------------------------------------------------------------------- */
typedef struct llnode llnode_t;
typedef struct llhead llhead_t;

struct llnode {
  llnode_t *prev_;
  llnode_t *next_;
};


struct llhead {
  llnode_t *first_;
  llnode_t *last_;
  int count_;
};

#define INIT_LLHEAD  {NULL,NULL,0}
#define INIT_LLNODE  {NULL,NULL}


/* ----------------------------------------------------------------------- */

/* Push an element at the end of a linked list */
static inline void ll_push_back(llhead_t *list, llnode_t *node)
{
  assert(node->prev_ == NULL);
  assert(node->next_ == NULL);

  node->prev_ = list->last_;
  if (list->last_ != NULL)
    list->last_->next_ = node;
  else
    list->first_ = node;

  node->next_ = NULL;
  list->last_ = node;
  ++list->count_;
}


/* Push an element at the front of a linked list */
static inline void ll_push_front(llhead_t *list, llnode_t *node)
{
  assert(node->prev_ == NULL);
  assert(node->next_ == NULL);

  node->next_ = list->first_;
  if (list->first_ != NULL)
    list->first_->prev_ = node;
  else
    list->last_ = node;

  node->prev_ = NULL;
  list->first_ = node;
  ++list->count_;
}


static inline llnode_t *ll_pop_back_(llhead_t *list)
{
  llnode_t *last = list->last_;
  if (last == 0)
    return NULL;

  assert(last->next_ == NULL);
  if (last->prev_) {
    last->prev_->next_ = NULL;
  } else {
    assert(list->first_ == last);
    list->first_ = NULL;
  }

  list->last_ = last->prev_;
  last->prev_ = NULL;
  last->next_ = NULL;
  --list->count_;
  return last;
}


/* Remove an item on the linked list
 * @note Don't check if the item is realy on the list. */
static inline void ll_remove(llhead_t *list, llnode_t *node)
{
#if !defined(NDEBUG)
  llnode_t *w = node;
  while (w->prev_) w = w->prev_;
  assert(w == list->first_);
#endif

  if (node->prev_) {
    node->prev_->next_ = node->next_;
  } else {
    assert(list->first_ == node);
    list->first_ = node->next_;
  }

  if (node->next_) {
    node->next_->prev_ = node->prev_;
  } else {
    assert(list->last_ == node);
    list->last_ = node->prev_;
  }

  node->prev_ = NULL;
  node->next_ = NULL;
  --list->count_;
}


static inline void ll_remove_if(llhead_t *list, llnode_t *node)
{
  if (node->next_ == NULL && node->prev_ == NULL && list->first_ != node)
    return;
  ll_remove(list, node);
}


#endif  /* _SKC_LLIST_H */
