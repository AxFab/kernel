#include <kernel/core.h>
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
  ck_assert (initialize_vfs() == 0);

  kCPU.ready_ = true;
  printf("VFS is ready\n");
}


// ---------------------------------------------------------------------------
void teardown ()
{
  kstat();
  ck_assert_msg (klockcount() == 0, "Some lock haven't been released!");
}


// ---------------------------------------------------------------------------
void *mmu_temporary (uint32_t *pg)
{
  if (*pg == 0)
    posix_memalign((void **)pg, PAGE_SIZE, PAGE_SIZE);

  return (void *) * pg;
}

int IMG_init (kInode_t *dev);
int ISO_mount (kInode_t *dev, kInode_t *mnt, const char *name);
// ===========================================================================
START_TEST (vfs_create)
{
  ck_assert (__geterrno() == 0);

  kobj_ok (EINVAL, create_inode(".config", kSYS.rootNd_, 0644, 0));

  kInode_t *ino1 = create_inode(".config", kSYS.rootNd_, 0644 | S_IFREG, 0);
  kobj_ok(0, ino1);

  kobj_ok (0, create_inode("stat", kSYS.rootNd_, 0644 | S_IFIFO, 0));
  kobj_ok (0, create_inode("logging", kSYS.rootNd_, 0644 | S_IFREG, 0));
  kobj_ok (0, create_inode(".archive", kSYS.rootNd_, 0644 | S_IFREG, 0));
  kobj_ok (EEXIST, create_inode("stat", kSYS.rootNd_, 0644 | S_IFIFO, 0));

  IMG_init(kSYS.devNd_);

  ck_assert (search_device(0) == NULL);
  ck_assert (search_device(1) != NULL);
  ck_assert (search_device(2) != NULL);
  ck_assert (search_device(3) == NULL);

  kobj_ok (0, search_inode("/.config", NULL));
  kobj_ok (ENOENT, search_inode("/dev/fb0", NULL));
  kobj_ok (0, search_inode("/dev/sdA", NULL));
  kobj_ok (ENOENT, search_inode("/dev/sdB", NULL));
  kobj_ok (0, search_inode("/dev/sdC", NULL));
  kobj_ok (ENOTDIR, search_inode("/dev/sdA/.ssh", NULL));
  kobj_ok (0, search_inode("/dev/../mnt", NULL));
  kobj_ok (ENOTDIR, search_inode("/dev/sdA/../../mnt", NULL));
  kInode_t *ino2 = search_inode("/dev", NULL);
  kobj_ok (0, ino2);
  kobj_ok (0, search_inode("./sdA", ino2));
  kobj_ok (0, search_inode("sdA", ino2));
  kobj_ok (ENOTDIR, search_inode("sdA/.", ino2));


  kInode_t *ino3 = search_inode("/dev/sdA", NULL);
  kobj_ok (0, ino3);
  kInode_t *ino4 = search_inode("/mnt", NULL);
  kobj_ok (0, ino4);
  // int fsIso = fs_find("iso");
  // kerr_ok(0, mount_device (ino3, "usr", kSYS.rootNd_, fsIso, 0, NULL));
  klock(&ino3->lock_);
  kerr_ok(0, ISO_mount (ino3, ino4, "cd"));
  kunlock(&ino3->lock_);

  kInode_t *ino5 = search_inode("/mnt/cd/BIN/", NULL);
  kobj_ok (0, ino5);

  kobj_ok (0, search_inode("MASTER.", ino5));

  kInode_t *ino6 = search_inode("./../BOOT/KIMAGE.MAP", ino5);
  kobj_ok (0, ino6);
  kobj_ok (ENOENT, search_inode("./../BOOT/CONFIG", ino5));

  uint32_t page = 0;
  kerr_ok(0, inode_open(ino6));
  kerr_ok(0, inode_page(ino6, 0, &page));
  kerr_ok(0, inode_page(ino6, 4096, &page));
  kerr_ok(0, inode_page(ino6, 8192, &page));
  kerr_ok(0, inode_page(ino6, 0, &page));
  kerr_ok(0, inode_page(ino6, 4096, &page));
  kerr_ok(0, inode_page(ino6, 8192, &page));

  int i;

  for (i = 0; i < ino6->stat_.length_ / PAGE_SIZE; ++i)
    kerr_ok(0, inode_page(ino6, 4096 * i, &page));

  kerr_ok(0, inode_page(ino6, 4096 * i, &page));
  ++i;
  kerr_ok(EINVAL, inode_page(ino6, 4096 * i, &page));

  kerr_ok(0, inode_close(ino6));

  kerr_ok(EINVAL, inode_open(NULL));
  kerr_ok(EINVAL, inode_close(NULL));

  scavenge_bucket(5);
  scavenge_inodes(500);
}
END_TEST


// ===========================================================================
void tests_tmpfs_api (Suite *suite)
{
  TCase *fixture = tcase_create ("VFS - tmpfs api");
  suite_add_tcase (suite, fixture);
  tcase_add_unchecked_fixture (fixture, setup, teardown);
  tcase_add_test (fixture, vfs_create);
}


// ---------------------------------------------------------------------------
int main (int argc, char **argv)
{
  Suite *suite = suite_create ("Kernel VFS unit-tests");

  tests_tmpfs_api (suite);

  SRunner *runner = srunner_create(suite);

  if (argc > 1) {
    srunner_set_log (runner, "report_vfsUT.log");
    srunner_set_xml (runner, "report_check_vfsUT.xml");
  }

  srunner_run_all (runner, CK_NORMAL);
  int failed_tests = srunner_ntests_failed(runner);
  srunner_free (runner);
  return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
