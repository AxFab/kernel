#include <skc/fifo.h>
#include <check.h>


START_TEST(test_matrix) {
  char buf[6];
  const char *abc = "abcdef";
  const char *ABC = "ABCDEF";
  fifo_t *fifo = fifo_init(&buf, 6);

  memcpy(buf, abc, 6);
  fifo_in(fifo, ABC, 3, FP_WR);
  ck_assert(!memcmp(buf, "ABCdef", 6));

} END_TEST

void fixture_fifo(Suite *s) {
  TCase *tc = tcase_create("FIFO");
  tcase_add_test(tc, test_matrix);
  suite_add_tcase(s, tc);
}
