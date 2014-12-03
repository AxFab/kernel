#include <kernel/core.h>
#include <kernel/memory.h>
#include <kernel/task.h>
#include <kernel/vfs.h>
#include <stdlib.h>
#include <stdio.h>
#include <check.h>

void kstat();
#define kerr_ok(e,c) do { ck_assert((e)?(c)!=0:(c)==0); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);
#define kobj_ok(e,c) do { ck_assert((e)?(c)==NULL:(c)!=NULL); ck_assert_msg(__geterrno() == e, "errno is '%s'", strerror(__geterrno())); } while(0);

// ---------------------------------------------------------------------------
void setup () 
{
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
void throw()
{
  ck_assert_msg(0, "Kernel Stack overflow");
}

// ===========================================================================
START_TEST (mem_miss) 
{
  kAddSpace_t mspace;
  memset(&mspace, 0, sizeof(mspace));
  kerr_ok (0, addspace_init (&mspace, 0));

  kSection_t section;
  kInode_t ino;
  ino.name_ = "axlibc.so";

  section.address_ = 0x1000000;
  section.length_ = 4 * PAGE_SIZE;
  section.flags_ = VMA_READ | VMA_EXEC | VMA_CODE;
  section.offset_ = 0x1000;
  kobj_ok (0, vmarea_map_section (&mspace, &section, &ino));

  section.address_ = 0x1004000;
  section.length_ = 3 * PAGE_SIZE;
  section.flags_ = VMA_READ | VMA_DATA;
  section.offset_ = 0x5000;
  kobj_ok (0, vmarea_map_section (&mspace, &section, &ino));

  section.address_ = 0x1007000;
  section.length_ = 6 * PAGE_SIZE;
  section.flags_ = VMA_READ | VMA_WRITE | VMA_DATA;
  section.offset_ = 0x8000;
  kobj_ok (0, vmarea_map_section (&mspace, &section, &ino));

  kVma_t* vma1 = vmarea_map(&mspace, 12 * _Kb_, VMA_STACK);
  kobj_ok (0, vma1);

  kVma_t* vma2 = vmarea_map(&mspace, 12 * _Mb_, VMA_HEAP);
  kobj_ok (0, vma2);

  kVma_t* vma3 = vmarea_map_at(&mspace, 12 * _Mb_, 1 * _Mb_ , VMA_SHM);
  kobj_ok (0, vma3);

  kobj_ok (EINVAL, vmarea_map(&mspace, 0 , VMA_SHM));
  kobj_ok (EINVAL, vmarea_map(&mspace, 0 , VMA_READ));
  kobj_ok (EINVAL, vmarea_map(&mspace, 5 * _Kb_ , VMA_READ));

  kobj_ok (EINVAL, vmarea_map_at(&mspace, (size_t)3 * _Gb_, (size_t)2 * _Gb_, VMA_SHM));
  kobj_ok (EINVAL, vmarea_map_at(&mspace, 1 * _Gb_, 2 * _Mb_, VMA_KERNEL));
  // kobj_ok (0, vmarea_map_at(&mspace, 3 * _Gb_, 2 * _Mb_, VMA_SHM | VMA_READ));

  kVma_t* vmaA = addspace_find(&mspace, 0x1000000 + 156);
  ck_assert (vmaA != NULL && vmaA->ino_ == &ino);

  // page_fault (0x1000000, PF_USER | PF_WRITE); 

  addspace_display (&mspace);
  // ck_assert (__geterrno() == 0);
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
