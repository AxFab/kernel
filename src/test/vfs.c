
#include <smkos/vfs.h>
#include <check.h>



START_TEST(test_fsinit) {
  vfs_init();
  vfs_display();
  vfs_sweep();
} END_TEST

void fixture_vfs(Suite *s) {
  TCase *tc = tcase_create("Virtual FileSystem");
  tcase_add_test(tc, test_fsinit);
  suite_add_tcase(s, tc);
}
