#include <skc/bbtree.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

typedef struct item {
  char *value_;
  bbnode_t node_;
}  item_t;

#define INIT_ITEM(i, n) {n, INIT_BBNODE(i)}

void bbnode_print(bbnode_t *node, int lvl) {
  int i;
  if (node == NULL)
    return;
  for (i=0; i < lvl; ++i)
    printf("  ");
  printf(" %d] %s\n", node->value_, itemof(node, item_t, node_)->value_);
  bbnode_print(node->left_, lvl + 1);
  bbnode_print(node->right_, lvl + 1);
}

void bbtree_print(bbtree_t *tree) {
  bbnode_print(tree->root_, 0); 
  printf("  -----\n");
}

START_TEST(test_bbbasic) {
  item_t a = INIT_ITEM(1, "#1");
  item_t b = INIT_ITEM(2, "#2");
  item_t c = INIT_ITEM(3, "#3");
  item_t d = INIT_ITEM(4, "#4");
  item_t e = INIT_ITEM(5, "#5");
  item_t *it;
  bbtree_t t = INIT_BBTREE;

  bb_insert(&t, &a.node_);
  bb_insert(&t, &b.node_);
  bb_insert(&t, &c.node_);
  bb_insert(&t, &d.node_);
  bbtree_print(&t);
  bb_remove(&t, &d.node_);
  it = bb_search(&t, 2, item_t, node_);
  ck_assert(it == &b && it->node_.value_ == 2 && !strcmp(it->value_, "#2"));
  bbtree_print(&t);
} END_TEST

START_TEST(test_dummy) {

  item_t a = INIT_ITEM(12,"Hen");
  item_t b = INIT_ITEM(4,"Dog");
  item_t c = INIT_ITEM(11,"Cat");
  item_t d = INIT_ITEM(1,"Frog");
  item_t e = INIT_ITEM(100,"Owner");
  item_t *it;
  bbtree_t t = INIT_BBTREE; // Create the binary tree

  //Insert data into the tree - key and some text
  bb_insert(&t, &a.node_);
  bb_insert(&t, &b.node_);
  bb_insert(&t, &c.node_);
  bb_insert(&t, &d.node_);
  bb_insert(&t, &e.node_);
  bbtree_print(&t);
  bb_remove(&t, &a.node_);
  it = bb_search(&t, 11, item_t, node_);
  ck_assert(it == &c && it->node_.value_ == 11 && !strcmp(it->value_, "Cat"));
  bbtree_print(&t);
} END_TEST



void fixture_bbtree(Suite *s) {
  TCase *tc = tcase_create("Self-Balanced Binary Tree");
  tcase_add_test(tc, test_bbbasic);
  tcase_add_test(tc, test_dummy);
  suite_add_tcase(s, tc);
}
