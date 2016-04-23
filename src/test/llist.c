#include <skc/llist.h>
#include <check.h>

typedef struct item {
  int value_;
  llnode_t node_;
}  item_t;

#define INIT_ITEM(i) {i, INIT_LLNODE}

START_TEST(test_llbasic) {
  item_t a = INIT_ITEM(1);
  item_t b = INIT_ITEM(2);
  item_t c = INIT_ITEM(3);
  item_t d = INIT_ITEM(4);
  item_t e = INIT_ITEM(5);
  item_t *it;
  llhead_t h = INIT_LLHEAD;

  ll_append(&h, &a.node_);
  ll_append(&h, &b.node_);
  ll_append(&h, &c.node_);
  ll_remove(&h, &b.node_);
  ll_append(&h, &d.node_);

  ll_foreach(&h, it, item_t, node_) {
    printf(" - %d", it->value_);
  }
  printf("\n");

  ll_remove(&h, &a.node_);
  ll_append(&h, &a.node_);
  ll_append(&h, &e.node_);

  ll_foreach(&h, it, item_t, node_) {
    printf(" - %d", it->value_);
  }
  printf("\n");

  ll_remove(&h, &e.node_);
  ll_remove(&h, &c.node_);
  ll_append(&h, &c.node_);

  ll_foreach(&h, it, item_t, node_) {
    printf(" - %d", it->value_);
  }
  printf("\n");

} END_TEST

void fixture_llist(Suite *s) {
  TCase *tc = tcase_create("LinkedList");
  tcase_add_test(tc, test_llbasic);
  suite_add_tcase(s, tc);
}
