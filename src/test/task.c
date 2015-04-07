#include <kernel/core.h>
#include <kernel/task.h>
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
  printf("TASK is ready\n");
}


// ---------------------------------------------------------------------------
void teardown ()
{
  kstat();
  ck_assert_msg (klockcount() == 0, "Some lock haven't been released!");
}

kInode_t *load_inode (const char *file);
kInode_t *build_inode_dir ();

int addspace_init(kAddSpace_t *space, int flags)
{
  return __seterrno(0);
}

kVma_t *vmarea_map(kAddSpace_t *space, size_t length, int flags)
{
  kVma_t *vma = KALLOC(kVma_t);
  vma->base_ = 8 * _Mb_;
  vma->limit_ = 8 * _Mb_ + length;
  vma->flags_ = flags;
  return vma;
}

kVma_t *vmarea_map_section(kAddSpace_t *space, kSection_t *section, kInode_t *ino)
{
  kVma_t *vma = KALLOC(kVma_t);
  vma->base_ = 4 * _Mb_;
  vma->limit_ = 4 * _Mb_ + section->length_;
  vma->flags_ = section->flags_;
  vma->offset_ = section->offset_;
  vma->ino_ = ino;
  return vma;
}

void *mmu_temporary (page_t *page)
{
  return (void *)(*page);
}

void sched_insert(kThread_t *thread)
{
  if (kCPU.current_ == NULL)
    kCPU.current_ = thread;

  // thread->nextSc_ = kCPU.current_;
  // kCPU.current_ = thread;
}


void sched_wakeup(kThread_t *thread)
{
}

void sched_remove(kThread_t *thread)
{
}


// ===========================================================================
START_TEST (task_create)
{
  ck_assert (__geterrno() == 0);

  kUser_t *usSys = create_user ("system", 0xfffff);
  kobj_ok(0, usSys);

  kUser_t *usGst = create_user ("guest", 0);
  kobj_ok(0, usGst);

  kobj_ok(EEXIST, create_user("guest", 0));

  kInode_t *ino = load_inode("bin/i686/debug/master");
  kInode_t *term = load_inode("Makefile");
  kInode_t *dir = build_inode_dir();

  kAssembly_t *image = load_assembly(ino);
  kobj_ok(0, image);
  kobj_ok(ENOEXEC, load_assembly(term));

  kProcess_t *proc1 = login_process(image, usSys, dir, term, "");
  kobj_ok(0, proc1);
  kobj_ok(0, append_thread(proc1, (void *)0x6990, 3));
  kThread_t *thread = append_thread(proc1, (void *)0x8f90, 3);
  kobj_ok(0, thread);

  thread->state_ = SCHED_ZOMBIE;
  kobj_ok(0, append_thread(proc1, (void *)0x6990, 3));
  kProcess_t *proc2 = create_process(image, "", "");
  kobj_ok(0, proc2);

  destroy_process(proc2);
  destroy_process(proc1);
  destroy_assembly(image);
  destroy_user(usGst);
  destroy_user(usSys);
}
END_TEST


// ===========================================================================
void tests_task (Suite *suite)
{
  TCase *fixture = tcase_create ("TASK - tasks");
  suite_add_tcase (suite, fixture);
  tcase_add_unchecked_fixture (fixture, setup, teardown);
  tcase_add_test (fixture, task_create);
}


// ---------------------------------------------------------------------------
int main (int argc, char **argv)
{
  Suite *suite = suite_create ("Kernel TASK unit-tests");

  tests_task (suite);

  SRunner *runner = srunner_create(suite);

  if (argc > 1) {
    srunner_set_log (runner, "report_taskUT.log");
    srunner_set_xml (runner, "report_check_taskUT.xml");
  }

  srunner_run_all (runner, CK_NORMAL);
  int failed_tests = srunner_ntests_failed(runner);
  srunner_free (runner);
  return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
