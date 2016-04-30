#include <check.h>
#include <stdlib.h>

void fixture_llist(Suite*);
void fixture_bbtree(Suite*);
void fixture_fifo(Suite*);


int main (int argc, char** argv) {
  Suite* suite = suite_create ("Kernel unit tests");

  fixture_llist(suite);
  fixture_bbtree(suite);
  fixture_fifo(suite);
  
  SRunner* runner = srunner_create(suite);
  if (argc > 1) {
    srunner_set_log (runner, "check.log");
    srunner_set_xml (runner, "report_check.xml");
  }

  srunner_run_all (runner, CK_NORMAL);
  int failed_tests = srunner_ntests_failed(runner);
  srunner_free (runner);
  return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
