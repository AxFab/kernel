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
 *      Implementation of a balanced binary-tree.
 */
#ifndef _SKC_BBTREE_H
#define _SKC_BBTREE_H 1

#include <skc/stddef.h>
#include <assert.h>

typedef struct bbtree bbtree_t;
typedef struct bbnode bbnode_t;


/* BBTree (self-balancing binary tree) node */
struct bbnode 
{
  struct bbnode *left_;
  struct bbnode *right_;
  size_t value_;
  int level_;
};

/* BBTree (self-balancing binary tree) head */
struct bbtree 
{
  struct bbnode *root_;
  int count_;
};

#define BBTREE_DEPTH_LMT 25
#define BBTREE_TREE_INITIALIZE { NULL, 0 }
#define BBTREE_NODE_INITIALIZE { NULL, NULL, 0, 0 }


#define bb_left(t,s,m)        (s*)pointerof(bb_left_((t)->root_),s,m)
#define bb_right(t,s,m)       (s*)pointerof(bb_right_((t)->root_),s,m)
#define bb_search(t,v,s,m)    (s*)pointerof(bb_search_((t)->root_,(v),BBTREE_DEPTH_LMT),s,m)
#define bb_search_le(t,v,s,m) (s*)pointerof(bb_search_le_((t)->root_,(v),BBTREE_DEPTH_LMT),s,m)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Get the node the most to the left (less). */
bbnode_t *bb_left_(bbnode_t *node);
/* Get the node the most to the right (more). */
bbnode_t *bb_right_(bbnode_t *node);
/* Get the node with is less or equal to the searching value. */
bbnode_t *bb_search_le_ (bbnode_t *node, size_t value, int limit);
/* Get the node with is equal to the searching value. */
bbnode_t *bb_search_ (bbnode_t *node, size_t value, int limit);
/* Add an item to the B-tree */
void bb_insert(bbtree_t *tree, bbnode_t *node);
/* Remove an item of the B-tree */
void bb_remove(bbtree_t *tree, bbnode_t *node);

#endif  /* _SKC_BBTREE_H */
