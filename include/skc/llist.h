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
#ifndef _SKC_LLIST_H
#define _SKC_LLIST_H 1

#include <skc/stddef.h>
#include <assert.h>

typedef struct llhead llhead_t;
typedef struct llnode llnode_t;


struct llhead 
{
  llnode_t *first_;
  llnode_t *last_;
  int count_;
};

struct llnode 
{
  llnode_t *prev_;
  llnode_t *next_;
};

#define LLIST_HEAD_INITIALIZE { NULL, NULL, 0 }
#define LLIST_NODE_INITIALIZE { NULL, NULL }


#define ll_append(l,n)        ll_push_back(l,n)
#define ll_enqueue(l,n)       ll_push_back(l,n)
#define ll_push(l,n)          ll_push_back(l,n)
#define ll_dequeue(l,t,m)     pointerof(ll_pop_front(l),t,m)
#define ll_pop(l,t,m)         pointerof(ll_pop_back(l),t,m)
#define ll_first(l,t,m)       pointerof((l)->first_,t,m)
#define ll_next(s,t,m)        pointerof((s)->m.next_,t,m)
#define ll_foreach(l,s,t,m)   for((s)=ll_first(l,t,m);(s);(s)=ll_next(s,t,m))
#define ll_used(l,n)          ((n)->prev_ != NULL || (n)->next_ != NULL || (l)->first_ == (n))

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Push an element at the front of a linked-list */
void ll_push_front(llhead_t *list, llnode_t *node);
/* Push an element at the end of a linked-list */
void ll_push_back(llhead_t *list, llnode_t *node);
/* Pop an element from the front of the linked-list */
llnode_t* ll_pop_front(llhead_t *list);
/* Pop an element from the end of the linked-list */
llnode_t* ll_pop_back(llhead_t *list);
/* Remove an item on the linked-list. */
void ll_remove(llhead_t *list, llnode_t *node);
/* Check if the node is include on the list. */
int ll_onthelist(llhead_t *list, llnode_t *node);

#endif  /* _SKC_LLIST_H */
