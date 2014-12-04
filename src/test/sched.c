#include <kernel/core.h>
#include <kernel/scheduler.h>
#include <stdlib.h>
#include <stdio.h>
#include <check.h>

void kstat();
#define kerr_ok(e,c) do { ck_assert((e)?(c)!=0:(c)==0); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);
#define kobj_ok(e,c) do { ck_assert((e)?(c)==NULL:(c)!=NULL); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);

// ---------------------------------------------------------------------------
void setup () 
{
  kCPU.ready_ = true;
  printf("SCHED is ready\n");
}


// ---------------------------------------------------------------------------
void teardown () 
{
  kstat();
  ck_assert_msg (klockcount() == 0, "Some lock haven't been released!");
}



// ===========================================================================
int addspace_init(kAddSpace_t* mspace, int flags)
{
  return __seterrno(0);
}

kVma_t* vmarea_map (kAddSpace_t* mspace, size_t length, int flags) 
{
  return __seterrno(0);
}



// ===========================================================================
START_TEST (sched_run) 
{
  ck_assert (__geterrno() == 0);


} 
END_TEST


// ===========================================================================
void tests_sched_tests (Suite* suite)
{
  TCase* fixture = tcase_create ("SCHED - tests");
  suite_add_tcase (suite, fixture);
  tcase_add_unchecked_fixture (fixture, setup, teardown);
  tcase_add_test (fixture, sched_run);
}


// ---------------------------------------------------------------------------
int main (int argc, char** argv) 
{
  Suite* suite = suite_create ("Kernel SCHED unit-tests");

  tests_sched_tests (suite);

  SRunner* runner = srunner_create(suite);
  if (argc > 1) {
    srunner_set_log (runner, "report_schedUT.log");
    srunner_set_xml (runner, "report_check_schedUT.xml");
  }
  srunner_run_all (runner, CK_NORMAL);
  int failed_tests = srunner_ntests_failed(runner);
  srunner_free (runner);
  return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;  
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
