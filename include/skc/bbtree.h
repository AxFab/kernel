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
 *      Self-Balanced Bianry tree implementation.
 */
#ifndef _SKC_BBTREE_H
#define _SKC_BBTREE_H  1

#include <cdefs/stddef.h>
#include <assert.h>


#define RECURS_LMT 64
#define RECURS_ERR() assert(0)

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define bb_best(t,s,m)        (s*)itemof(bb_best_((t)->root_),s,m)
#define bb_search(t,v,s,m)    (s*)itemof(bb_search_((t)->root_,(v),RECURS_LMT),s,m)
#define bb_search_le(t,v,s,m) (s*)itemof(bb_search_le_((t)->root_,(v),RECURS_LMT),s,m)

#define bb_search_less(t,v,s,m) (s*)itemof(bb_search_le_((t)->root_,(v),RECURS_LMT),s,m)
#define bb_full_left(n,s,m) (s*)itemof(bb_full_left_(n),s,m)
#define bb_next(n,s,m) (s*)itemof(bb_next_(n),s,m)
#define bb_previous(n,s,m) (s*)itemof(bb_previous_(n),s,m)

/* ----------------------------------------------------------------------- */
typedef struct bbtree bbtree_t;
typedef struct bbnode bbnode_t;


/* BBTree (self-balancing binary tree) head */
struct bbtree {
  bbnode_t *root_;
  int count_;
};

/* BBTree (self-balancing binary tree) node */
struct bbnode {
  bbnode_t *parent_;
  bbnode_t *left_;
  bbnode_t *right_;
  size_t value_;
  int level_;
};

#define INIT_BBTREE     {NULL,0}
#define INIT_BBNODE(n)  {NULL,NULL,NULL,n,0}



/* ----------------------------------------------------------------------- */

/* Swap the pointers of horizontal left links.
 *         |             |
 *    L <- T             L -> T
 *   / \    \     =>    /    / \
 *  A   B    R         A    B   R
 */
static inline bbnode_t *bb_skew(bbnode_t *node)
{
  bbnode_t *temp;

  if (node == NULL || node->left_ == NULL ||
      node->left_->level_ != node->level_)
    return node;

  temp = node;
  node->parent_ = temp;
  temp->parent_ = node;
  node = node->left_;
  temp->left_ = node->right_;
  if (node->right_ != NULL) {
    node->right_->parent_ = temp;
  }
  node->right_ = temp;
  return node;
}


/* If we have two horizontal right links.
 * Take the middle node, elevate it, and return it.
 *    |                      |
 *    T -> R -> X            R
 *   /    /         =>      / \
 *  A    B                 T   X
 *                        / \
 *                       A   B
 */
static inline bbnode_t *bb_split(bbnode_t *node)
{
  bbnode_t *temp;

  if (node == NULL || node->right_ == NULL ||
      node->right_->right_ == NULL ||
      node->level_ != node->right_->right_->level_)
    return node;

  temp = node;
  node->parent_ = temp;
  temp->parent_ = node;
  node = node->right_;
  temp->right_ = node->left_;
  temp->left_ = node->right_;
  if (node->left_ != NULL) {
    node->left_->parent_ = temp;
  }
  node->left_ = temp;
  ++node->level_;
  return node;
}

/* */
static inline bbnode_t *bb_insert_(bbnode_t *root, bbnode_t *node, int limit)
{
  if (--limit < 0) RECURS_ERR();

  if (root == NULL) {
    node->level_ = 1;
    node->parent_ = NULL;
    node->right_ = NULL;
    node->left_ = NULL;
    return node;
  }

  if (node->value_ < root->value_) {
    root->left_ = bb_insert_ (root->left_, node, limit - 1);
    root->left_->parent_ = root;
  } else { /* if (node->value_ > root->value_) { */
    root->right_ = bb_insert_ (root->right_, node, limit - 1);
    root->right_->parent_ = root;
  }

  root = bb_skew(root);
  root = bb_split(root);
  return root;
}


static inline  bbnode_t *bb_decrease_lvl(bbnode_t *node)
{
  int lvl = 0;

  if (node->left_)
    lvl = MIN (lvl, node->left_->level_);

  if (node->right_)
    lvl = MIN (lvl, node->right_->level_);

  ++lvl;

  if (lvl < node->level_) {
    node->level_ = lvl;

    if (node->right_ && lvl < node->right_->level_)
      node->right_->level_ = lvl;
  }

  return node;
}

static inline bbnode_t *bb_remove_(bbtree_t *tree, bbnode_t *node, size_t rmVal, int limit)
{
  bbnode_t *rplc = NULL;

  if (--limit < 0) RECURS_ERR();

  if (node == NULL)
    return NULL;

  if (rmVal < node->value_) {
    node->left_ = bb_remove_ (tree, node->left_, rmVal, limit - 1);
  } else if (rmVal > node->value_) {
    node->right_ = bb_remove_(tree, node->right_, rmVal, limit - 1);
  } else {
    /* If we're a leaf, easy, otherwise reduce to leaf case. */
    if (node->left_ == NULL && node->right_ == NULL) {
      return NULL;
    } else if (node->left_ == NULL) {
      rplc = node->right_; /* Successor !? */
      rplc->right_ = bb_remove_ (tree, node->right_->right_, rplc->value_, limit - 1);
    } else if (node->right_ == NULL) {
      rplc = node->left_; /* Predecessor !? -- This one may be bugged. */
      rplc->left_ = bb_remove_ (tree, node->left_->left_, rplc->value_, limit - 1);
    } else {
      rplc = node->left_;
    }

    node = rplc;
  }

  if (node->left_ != NULL) {
    node->left_->parent_ = node;
  }
  if (node->right_ != NULL) {
    node->right_->parent_ = node;
  }

  /* Rebalance the tree. Decrease the level of all nodes in this level if
  necessary, and then skew and split all nodes in the new level. */
  node = bb_decrease_lvl (node);
  node = bb_skew (node);
  node->right_ = bb_skew(node->right_);

  if (node->right_ != NULL)
    node->right_->right_ = bb_skew(node->right_->right_);

  node = bb_split(node);
  node->right_ = bb_split(node->right_);
  return node;
}

/* Look for the closest left adjacent node */
static inline bbnode_t *bb_search_le_(bbnode_t *root, size_t value, int limit)
{
  bbnode_t *best;

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

/* Look for an exact value match */
static inline bbnode_t *bb_search_(bbnode_t *root, size_t value, int limit)
{
  if (--limit < 0) RECURS_ERR();

  if (root == NULL)
    return NULL;

  if (root->value_ > value)
    return bb_search_(root->left_, value, limit - 1);
  else if (root->value_ < value)
    return bb_search_(root->right_, value, limit - 1);

  return root;
}

/* Find the node on extreme left side  */
static inline bbnode_t *bb_left_(bbnode_t *node)
{
  if (node == NULL)
    return NULL;

  while (node->left_ != NULL)
    node = node->left_;

  return node;
}

/* Find the node on extreme right side  */
static inline bbnode_t *bb_right_(bbnode_t *node)
{
  if (node == NULL)
    return NULL;

  while (node->right_ != NULL)
    node = node->right_;

  return node;
}

/* Insert a new node on the tree */
static inline void bb_insert(bbtree_t *tree, bbnode_t *node)
{
  tree->root_ = bb_insert_(tree->root_, node, RECURS_LMT);
  tree->count_++;
}

/* Remove a node from the tree */
static inline void bb_remove(bbtree_t *tree, bbnode_t *node)
{
  tree->root_ = bb_remove_(tree, tree->root_, node->value_, RECURS_LMT);
  tree->count_--;
}

/* ----------------------------------------------------------------------- */

static inline void *bb_full_left_(bbnode_t *root)
{
  bbnode_t *node = root;
  if (node == NULL) {
    return NULL;
  }

  while (node->left_ != NULL) {
    node = node->left_;
  }

  return node;
}

static inline void *bb_full_right_(bbnode_t *root)
{
  bbnode_t *node = root;
  if (node == NULL) {
    return NULL;
  }

  while (node->left_ != NULL) {
    node = node->left_;
  }

  return node;
}

static inline void *bb_previous_(bbnode_t *root)
{
  bbnode_t *node = root;
  if (root->left_ != NULL) {
    return bb_full_right_(root->left_);
  }

  for (; node->parent_ !=NULL ; node = node->parent_) {
    if (node->parent_->left_ != node) {
      return bb_full_right_(node->parent_);
    }
  }

  return NULL;
}

static inline void *bb_next_(bbnode_t *root)
{
  bbnode_t *node = root;
  if (root->right_ != NULL) {
    return bb_full_left_(root->right_);
  }

  for (; node->parent_ !=NULL ; node = node->parent_) {
    if (node->parent_->right_ != node) {
      return bb_full_left_(node->parent_);
    }
  }

  return NULL;
}


#endif  /* _SKC_BBTREE_H */
