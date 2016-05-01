#include <skc/fifo.h>
#include <check.h>


START_TEST(test_matrix) {
  char buf[6];
  char tmp[6];
  const char *abc = "abcdef";
  const char *ABC = "ABCDEF";
  fifo_t *fifo = fifo_init(&buf, 6);

  memcpy(buf, abc, 6);
  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 3, FP_NOBLOCK) == 0);
  ck_assert(!memcmp(tmp, "\0\0\0\0\0\0", 6));

  ck_assert(fifo_in(fifo, "ABC", 3, FP_WR) == 3);
  ck_assert(!memcmp(buf, "ABCdef", 6));

  ck_assert(fifo_out(fifo, tmp, 2, FP_NOBLOCK) == 2);
  ck_assert(!memcmp(tmp, "AB\0\0\0\0", 6));

  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 3, FP_NOBLOCK) == 1);
  ck_assert(!memcmp(tmp, "C\0\0\0\0\0", 6));

  ck_assert(fifo_in(fifo, "DEFG", 4, FP_WR) == 4);
  ck_assert(!memcmp(buf, "GBCDEF", 6));

  ck_assert(fifo_in(fifo, "HIJK", 4, FP_WR) == 2);
  ck_assert(!memcmp(buf, "GHIDEF", 6));

  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 4, FP_NOBLOCK) == 4);
  ck_assert(!memcmp(tmp, "DEFG\0\0", 6));


  fifo_reset(fifo);
  memcpy(buf, abc, 6);

  ck_assert(fifo_in(fifo, "AB", 2, FP_WR) == 2);
  ck_assert(!memcmp(buf, "ABcdef", 6));

  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 2, FP_NOBLOCK | FP_EOL) == 0);
  ck_assert(!memcmp(tmp, "\0\0\0\0\0\0", 6));

  ck_assert(fifo_in(fifo, "C\n", 2, FP_WR) == 2);
  ck_assert(!memcmp(buf, "ABC\nef", 6));

  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 2, FP_NOBLOCK | FP_EOL) == 2);
  ck_assert(!memcmp(tmp, "AB\0\0\0\0", 6));

  ck_assert(fifo_in(fifo, "EF", 2, FP_WR) == 2);
  ck_assert(!memcmp(buf, "ABC\nEF", 6));

  memset(tmp, 0, 6);
  ck_assert(fifo_out(fifo, tmp, 4, FP_NOBLOCK | FP_EOL) == 2);
  ck_assert(!memcmp(tmp, "C\n\0\0\0\0", 6));


} END_TEST

void fixture_fifo(Suite *s) {
  TCase *tc = tcase_create("FIFO");
  tcase_add_test(tc, test_matrix);
  suite_add_tcase(s, tc);
}
