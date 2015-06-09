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
 *      Implementation of a double linked list.
 */
#pragma once
#include <stddef.h>
#include <smkos/assert.h>

#ifndef offsetof
#  define offsetof(t,m)   (size_t)&(((t*)0)->m)
#  define itemof(p,t,m)   (t*)((char*)p-offsetof(t,m))
#endif

#define ll_append(h,n)        ll_push_back(h,n)
#define ll_next(v,t,m)        (t*)ll_next_((v),offsetof(t,m))
#define ll_enqueue(h,n)       ll_push_front(h,n)
#define ll_dequeue(h,t,m)     ll_pop_back(h,t,m)
#define ll_pop_back(h,t,m)    (t*)ll_pop_back_((h),offsetof(t,m))
#define ll_first(h,t,m)       (t*)ll_first_((h),offsetof(t,m))
#define ll_for_each(h,v,t,m)  \
  for ((v)=ll_first(h,t,m);(v);(v)=(t*)ll_next(v,t,m))



/* ----------------------------------------------------------------------- */
struct llnode {
  struct llnode *prev_;
  struct llnode *next_;
};


/* ----------------------------------------------------------------------- */
struct llhead {
  struct llnode *first_;
  struct llnode *last_;
  int count_;
};

#define INIT_LLHEAD  {NULL,NULL,0}


/* ----------------------------------------------------------------------- */
/** Push an element at the end of a linked list */
static inline void ll_push_back(struct llhead *list, struct llnode *node)
{
  assert (node->prev_ == NULL);
  assert (node->next_ == NULL);

  node->prev_ = list->last_;

  if (list->last_ != NULL)
    list->last_->next_ = node;
  else
    list->first_ = node;

  node->next_ = NULL;
  list->last_ = node;
  ++list->count_;
}


/* ----------------------------------------------------------------------- */
/** Push an element at the front of a linked list */
static inline void ll_push_front(struct llhead *list, struct llnode *node)
{
  assert (node->prev_ == NULL);
  assert (node->next_ == NULL);

  node->next_ = list->first_;

  if (list->first_ != NULL)
    list->first_->prev_ = node;
  else
    list->last_ = node;

  node->prev_ = NULL;
  list->first_ = node;
  ++list->count_;
}


/* ----------------------------------------------------------------------- */
/**
  */
static inline void *ll_pop_back_(struct llhead *list, size_t off)
{
  struct llnode *last = list->last_;

  if (last == 0)
    return NULL;

  assert (last->next_ == NULL);

  if (last->prev_) {
    last->prev_->next_ = NULL;
  } else {
    assert (list->first_ == last);
    list->first_ = NULL;
  }

  list->last_ = last->prev_;
  last->prev_ = NULL;
  last->next_ = NULL;
  --list->count_;

  return (char *)last - off;
}


/* ----------------------------------------------------------------------- */
/** Remove an item on the linked list
  * @note Don't check if the item is realy on the list.
 */
static inline void ll_remove(struct llhead *list, struct llnode *node)
{
#if !defined(NDEBUG)
  struct llnode *w = node;

  while (w->prev_) w = w->prev_;

  assert (w == list->first_);
#endif

  if (node->prev_) {
    node->prev_->next_ = node->next_;
  } else {
    assert (list->first_ == node);
    list->first_ = node->next_;
  }

  if (node->next_) {
    node->next_->prev_ = node->prev_;
  } else {
    assert (list->last_ == node);
    list->last_ = node->prev_;
  }

  node->prev_ = NULL;
  node->next_ = NULL;
  --list->count_;
}


/* ----------------------------------------------------------------------- */
static inline void ll_remove_if(struct llhead *list, struct llnode *node)
{
  if (node->next_ == NULL && node->prev_ == NULL && list->first_ != node)
    return;

  ll_remove(list, node);
}


/* ----------------------------------------------------------------------- */
static inline void *ll_first_(struct llhead *list, size_t off)
{
  if (!list->first_)
    return NULL;

  return (char *)list->first_ - off;
}


/* ----------------------------------------------------------------------- */
static inline void *ll_next_(void *item, size_t off)
{
  struct llnode *node = (struct llnode *)((char *)item + off);

  if (!node->next_)
    return NULL;

  return (char *)node->next_ - off;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
