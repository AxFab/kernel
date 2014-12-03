#ifndef KERNEL_AATREE_H__
#define KERNEL_AATREE_H__

#include <kernel/core.h>
#include <kernel/spinlock.h>

#define AA_RECURS_LIMIT   25

void throw();

// ---------------------------------------------------------------------------
/** Swap the pointers of horizontal left links. */
static inline aanode_t* aa_skew(aanode_t* node) 
{
  if (node == NULL || node->left_ == NULL || 
      node->left_->level_ != node->level_) 
    return node;

  aanode_t* temp = node;
  node = node->left_;
  temp->left_ = node->right_;
  node->right_ = temp;
  return node;
}


// ---------------------------------------------------------------------------
/** If we have two horizontal right links.
  * Take the middle node, elevate it, and return it.
  */
static inline aanode_t* aa_split(aanode_t* node) 
{
  if (node == NULL || node->right_ == NULL || 
      node->right_->right_ == NULL || 
      node->level_ != node->right_->right_->level_) 
    return node;

  aanode_t* temp = node;
  node = node->right_;
  temp->right_ = node->left_;
  node->left_ = temp;
  ++node->level_;
  return node;
}


// ---------------------------------------------------------------------------
static inline aanode_t* aa_insert_(aanode_t* root, aanode_t* node, int limit)
{
  if (--limit < 0) throw();
  if (root == NULL) {
    node->level_ = 1;
    node->right_ = NULL;
    node->left_ = NULL;
    return node;
  } 

  if (node->value_ < root->value_) {
    root->left_ = aa_insert_ (root->left_, node, limit-1);
  } else { // if (node->value_ > root->value_) { 
    root->right_ = aa_insert_ (root->right_, node, limit-1);
  } 

  root = aa_skew(root);
  root = aa_split(root);
  return root;
}


// ---------------------------------------------------------------------------
static inline aanode_t* aa_delete_(aatree_t* tree, aanode_t* root, aanode_t* node, int limit)
{
  if (--limit < 0) throw();
  if (root == NULL)
    return NULL;

  // Search down the tree and set pointers last and deleted
  tree->last_ = root;
  if (node->value_ < root->value_) {
    root->left_ = aa_delete_ (tree, root->left_, node, limit-1);
  } else { // if (node->value_ > root->value_) { 
    tree->deleted_ = root;
    root->right_ = aa_delete_(tree, root->right_, node, limit-1);
  } 

  // At the bottom of the tree we remove the element (if it is present)
  if (tree->last_ == root && tree->deleted_ != NULL && node == tree->deleted_) {
    tree->deleted_->value_ = root->value_;
    tree->deleted_ = NULL;
    root = root->right_;
    // dispose (last);
    
  // On the way back, we rebalance
  } else {

    long lvl = 0;
    if (node->left_) lvl = MIN (lvl, node->left_->level_);
    if (node->right_) lvl = MIN (lvl, node->right_->level_);

    if (lvl < root->level_ - 1) {
      root->level_ = root->level_ - 1;
      if (root->right_ != NULL && root->right_->level_ > root->level_)
        root->right_->level_ = root->level_;
      root = aa_skew(root);
      root->right_ = aa_skew(root->right_);
      if (root->right_ != NULL)
        root->right_->right_ = aa_skew(root->right_->right_);
      root = aa_split(root);
      root->right_ = aa_split(root->right_);
    }
  }

  return root; 
}


// ---------------------------------------------------------------------------
static inline aanode_t* aa_search_lesseq_ (aanode_t* root, long value, int limit)
{
  aanode_t* best;
  if (--limit < 0) throw();
  if (root == NULL)
    return NULL;

  if (root->value_ > value) 
    return aa_search_lesseq_(root->left_, value, limit-1);

  best = aa_search_lesseq_(root->right_, value, limit-1);
  if (best != NULL && root->value_ < best->value_)
    return best;
  return root;
}


// ---------------------------------------------------------------------------
static inline aanode_t* aa_best (aatree_t* tree) 
{
  klock(&tree->lock_);
  aanode_t* node = tree->root_;
  if (node == NULL) {
    kunlock(&tree->lock_);
    return NULL;
  }
  
  while (node->left_ != NULL)
    node = node->left_;
  kunlock(&tree->lock_);
  return node;
}


// ---------------------------------------------------------------------------
static inline void aa_insert (aatree_t* tree, aanode_t* node) 
{
  klock(&tree->lock_);
  tree->root_ = aa_insert_(tree->root_, node, AA_RECURS_LIMIT);
  kunlock(&tree->lock_);
} 


// ---------------------------------------------------------------------------
static inline void aa_delete (aatree_t* tree, aanode_t* node) 
{
  klock(&tree->lock_);
  tree->root_ = aa_delete_(tree, tree->root_, node, AA_RECURS_LIMIT);
  kunlock(&tree->lock_);
} 


// ---------------------------------------------------------------------------
static inline aanode_t* aa_search_lesseq (aatree_t* tree, long value) 
{
  klock(&tree->lock_);
  aanode_t* node = aa_search_lesseq_(tree->root_, value, AA_RECURS_LIMIT);
  kunlock(&tree->lock_);
  return node;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#endif /* KERNEL_AATREE_H__ */
