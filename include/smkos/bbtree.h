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
 *      Implementation of a balanced binary tree: AA-tree.
 */
#pragma once
#include <stddef.h>
#include <smkos/_assert.h>

#define RECURS_LMT 64
#define RECURS_ERR() assert(0)

#ifndef offsetof
#  define offsetof(t,m)   (size_t)&(((t*)0)->m)
#endif

#ifndef itemof
#  define itemof(p,t,m)   (t*)item_(p,offsetof(t,m))
static inline void* item_(void* p, size_t off)
{
  if (p == NULL)
    return NULL;
  return ((char*)p - off);
}
#endif

#define bb_best(t,s,m)        (s*)itemof(bb_best_((t)->root_),s,m)
#define bb_search(t,v,s,m)    (s*)itemof(bb_search_((t)->root_,(v),RECURS_LMT),s,m)
#define bb_search_le(t,v,s,m) (s*)itemof(bb_search_le_((t)->root_,(v),RECURS_LMT),s,m)


/* ----------------------------------------------------------------------- */
/** BBTree (self-balancing binary tree) node */
struct bbnode {
  struct bbnode *left_;
  struct bbnode *right_;
  size_t value_;
  int level_;
};


/* ----------------------------------------------------------------------- */
/** BBTree (self-balancing binary tree) head */
struct bbtree {
  struct bbnode *root_;
  struct bbnode *last_;
  struct bbnode *deleted_;
  int count_;
};

#define INIT_BBNODE  {NULL,NULL,0,0}
#define INIT_BBTREE  {NULL,NULL,NULL,0}


/* ----------------------------------------------------------------------- */
/** Swap the pointers of horizontal left links. */
static inline struct bbnode *bb_skew(struct bbnode *node) {
  struct bbnode *temp;

  if (node == NULL || node->left_ == NULL ||
      node->left_->level_ != node->level_)
    return node;

  temp = node;
  node = node->left_;
  temp->left_ = node->right_;
  node->right_ = temp;
  return node;
}


/* ----------------------------------------------------------------------- */
/** If we have two horizontal right links.
  * Take the middle node, elevate it, and return it.
 */
static inline struct bbnode *bb_split(struct bbnode *node) {
  struct bbnode *temp;

  if (node == NULL || node->right_ == NULL ||
      node->right_->right_ == NULL ||
      node->level_ != node->right_->right_->level_)
    return node;

  temp = node;
  node = node->right_;
  temp->right_ = node->left_;
  node->left_ = temp;
  ++node->level_;
  return node;
}


/* ----------------------------------------------------------------------- */
static inline struct bbnode *bb_insert_(struct bbnode *root, struct bbnode *node, int limit) {
  if (--limit < 0) RECURS_ERR();

  if (root == NULL) {
    node->level_ = 1;
    node->right_ = NULL;
    node->left_ = NULL;
    return node;
  }

  if (node->value_ < root->value_) {
    root->left_ = bb_insert_ (root->left_, node, limit - 1);
  } else { /* if (node->value_ > root->value_) { */
    root->right_ = bb_insert_ (root->right_, node, limit - 1);
  }

  root = bb_skew(root);
  root = bb_split(root);
  return root;
}


/* ----------------------------------------------------------------------- */
static inline struct bbnode *bb_delete_(struct bbtree *tree, struct bbnode *root, struct bbnode *node, int limit) {
  if (--limit < 0) RECURS_ERR();

  if (root == NULL)
    return NULL;

  /* Search down the tree and set pointers last and deleted */
  tree->last_ = root;

  if (node->value_ < root->value_) {
    root->left_ = bb_delete_ (tree, root->left_, node, limit - 1);
  } else { /* if (node->value_ > root->value_) { */
    tree->deleted_ = root;
    root->right_ = bb_delete_(tree, root->right_, node, limit - 1);
  }

  /* At the bottom of the tree we remove the element (if it is present) */
  if (tree->last_ == root && tree->deleted_ != NULL && node == tree->deleted_) {
    tree->deleted_->value_ = root->value_;
    tree->deleted_ = NULL;
    root = root->right_;
    /* dispose (last); */

    /* On the way back, we rebalance */
  } else {

    int lvl = 0;

    if (node->left_) lvl = MIN (lvl, node->left_->level_);

    if (node->right_) lvl = MIN (lvl, node->right_->level_);

    if (lvl < root->level_ - 1) {
      root->level_ = root->level_ - 1;

      if (root->right_ != NULL && root->right_->level_ > root->level_)
        root->right_->level_ = root->level_;

      root = bb_skew(root);
      root->right_ = bb_skew(root->right_);

      if (root->right_ != NULL)
        root->right_->right_ = bb_skew(root->right_->right_);

      root = bb_split(root);
      root->right_ = bb_split(root->right_);
    }
  }

  return root;
}


/* ----------------------------------------------------------------------- */
static inline struct bbnode *bb_search_le_ (struct bbnode *root, size_t value, int limit) {
  struct bbnode *best;

  if (--limit < 0) RECURS_ERR();

  if (root == NULL)
    return NULL;

  if (root->value_ > value)
    return bb_search_le_(root->left_, value, limit - 1);

  best = bb_search_le_(root->right_, value, limit - 1);

  if (best != NULL && root->value_ < best->value_)
    return best;

  return root;
}

/* ----------------------------------------------------------------------- */
static inline struct bbnode *bb_search_ (struct bbnode *root, size_t value, int limit) {
  if (--limit < 0) RECURS_ERR();

  if (root == NULL)
    return NULL;

  if (root->value_ > value)
    return bb_search_(root->left_, value, limit - 1);
  else if (root->value_ < value)
    return bb_search_(root->right_, value, limit - 1);

  return root;
}


/* ----------------------------------------------------------------------- */
static inline struct bbnode *bb_best_ (struct bbnode *node) {
  if (node == NULL) {
    return NULL;
  }

  while (node->left_ != NULL)
    node = node->left_;

  return node;
}


/* ----------------------------------------------------------------------- */
static inline void bb_insert (struct bbtree *tree, struct bbnode *node)
{
  tree->root_ = bb_insert_(tree->root_, node, RECURS_LMT);
}


/* ----------------------------------------------------------------------- */
static inline void bb_delete (struct bbtree *tree, struct bbnode *node)
{
  tree->root_ = bb_delete_(tree, tree->root_, node, RECURS_LMT);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
