#include <kernel/core.h>
#include <kernel/memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <check.h>

void kstat();
#define kerr_ok(e,c) do { ck_assert((e)?(c)!=0:(c)==0); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);
#define kobj_ok(e,c) do { ck_assert((e)?(c)==NULL:(c)!=NULL); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);

// ---------------------------------------------------------------------------
void setup () 
{
  kvma_init();
  // TODO Set UP Boundaries
  // TODO Use  page_fault to check if we can do something like WRITE / EXEC / READ....
  // Goal is to map file and check with page_fault is we can RWX on it...
  // We should have something with inode too.... create an inode form file...

  kCPU.ready_ = true;
  printf("MEM is ready\n");
}


// ---------------------------------------------------------------------------
void teardown () 
{
  kstat();
  ck_assert_msg (klockcount() == 0, "Some lock haven't been released!");
}


// ===========================================================================
START_TEST (mem_miss) 
{
  kAddSpace_t space;
  kerr_ok (0, addspace_init (&space, 0));

  kVma_t vma0 = {0, 0, 12 * _Kb_, NULL, NULL, NULL, 0 };
  kVma_t* vma1  = kvma_mmap(&space, &vma0);

  vma0.base_ = 12 * _Mb_;
  kVma_t* vma2  = kvma_mmap(&space, &vma0);

  ck_assert (__geterrno() == 0);
} 
END_TEST


// ===========================================================================
void tests_pages (Suite* suite)
{
  TCase* fixture = tcase_create ("MEM - pages");
  suite_add_tcase (suite, fixture);
  tcase_add_unchecked_fixture (fixture, setup, teardown);
  tcase_add_test (fixture, mem_miss);
}


// ---------------------------------------------------------------------------
int main (int argc, char** argv) 
{
  Suite* suite = suite_create ("Kernel MEM unit-tests");

  tests_pages (suite);

  SRunner* runner = srunner_create(suite);
  if (argc > 1) {
    srunner_set_log (runner, "report_memUT.log");
    srunner_set_xml (runner, "report_check_memUT.xml");
  }
  srunner_run_all (runner, CK_NORMAL);
  int failed_tests = srunner_ntests_failed(runner);
  srunner_free (runner);
  return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;  
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
