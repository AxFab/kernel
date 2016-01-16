#include <smkos/kapi.h>
#include <smkos/alimits.h>
#include <smkos/kstruct/user.h>
#include <smkos/kstruct/map.h>
#include <smkos/drivers.h>

void kernel_info();
void ksymbols_load (kInode_t *ino);
void ksymclean();

struct spinlock *SP_first = NULL, *SP_last = NULL;

static const char *masterPaths[] = {
  "sbin/master.xe",
  "bin/master.xe",
  "master.xe",
  "sbin/master",
  "bin/master",
  "master",
  NULL
};


/* ----------------------------------------------------------------------- */
void kernel_ready ()
{
  kprintf ("CPU %d is ready\n", kCpuNo);
  // for (;;);
  // while (!kSYS.ready_) __delayX(10000);
}


/* ----------------------------------------------------------------------- */
void kernel_start ()
{
  int idx;
  kUser_t *user;
  kInode_t *ino;
  kInode_t *kb;
  kInode_t *fb;
  struct tm dateTime;

  /* Initialize kernel environment */
  atomic_t avail = kSYS.pageAvailable_;
  size_t max = kSYS.memMax_;
  int pag = kSYS.pageMax_;
  atomic_t used = kSYS.pageUsed_;
  memset (&kSYS, 0, sizeof (kSYS));
  memset (&kCPU, 0, sizeof (kCPU));
  kSYS.pageAvailable_ = avail;
  kSYS.memMax_ = max;
  kSYS.pageMax_ = pag;
  kSYS.pageUsed_ = used;

  kernel_state(KST_KERNSP);
  kprintf("SmokeOS " _VTAG_ ", build at " __DATE__ ".\n");
  mmu_load_env();

  /* Initialize time managment */
  dateTime = cpu_get_clock();
  kprintf("Date: %s\n", asctime(&dateTime));

  initialize_smp();

  /* Initialize the VFS */
  initialize_vfs();

  /* Search kernel helper files */
  ino = search_inode("boot/kImage.map", kSYS.sysIno_, 0, NULL);

  if (ino)
    ksymbols_load(ino);

  /* Create basic users */
  user = create_user("system", CAP_SYSTEM);
  assert_msg(user, "Unable to create system user.");

  /* Load master program */
  idx = 0;

  while (masterPaths[idx]) {
    ino = search_inode (masterPaths[idx], kSYS.sysIno_, 0, NULL);

    if (ino && NULL != load_assembly(ino))
      break;

    ++idx;
  }

  // Open Graphic buffer
  kb = search_inode ("/dev/Kb0", NULL, 0, NULL);
  fb = search_inode ("/dev/Fb0", NULL, 0, NULL);
  create_subsys(kb, fb);

  if (!masterPaths[idx])
    kpanic("Unable to find startup program 'MASTER'\n");

  create_logon_process(ino, user, kSYS.sysIno_, masterPaths[idx]);
  scavenge_area(kSYS.mspace_);

  // kprintf ("CPU %d is ready\n", kCpuNo);
  kernel_info();
  cpu_start_scheduler();

}


/* ----------------------------------------------------------------------- */
/** @brief Clean all unused item.
  *
  * This is mostly for memory checks and debuging tools.
  * The sweep can also be used to refresh kernel data and flush all cache.
  * Implementation can not guarantee a full cleaning.
  */
void kernel_sweep()
{
  kprintf ("\x1b[31mEnding...\x1b[0m\n");
  clean_subsys();
  scavenge_inodes(8000);
  scavenge_area(kSYS.mspace_);

  if (kSYS.mspace_->vrtPages_ != 0) {
    kprintf("/!\\ Kernel pages are leaking...\n");
    area_display(kSYS.mspace_);
  }

  sweep_vfs();
  destroy_all_users();
  ksymclean();
  mmu_leave_env();
}

/* ----------------------------------------------------------------------- */
void kernel_info()
{
  int i;
  struct tm dateTime;
  dateTime = cpu_get_clock();
  kprintf("\033[38mSmokeOS " _VTAG_ "\033[0m, build at " __DATE__ " from git:" _GITH_ ".\n\n");
  kprintf("Date: %s\n", asctime(&dateTime));
  kprintf("Cpus count: %d\n", kSYS.cpuCount_);
  for (i=0; i<kSYS.cpuCount_; ++i)
    kprintf("  - CPU %d :: %s\n", i, kSYS._cpu[0].spec_);
  kprintf ("Memory: ");
  kprintf (" %s detected, ", kpsize((uintmax_t)kSYS.memMax_));
  kprintf (" %s allocatable, ", kpsize((uintmax_t)kSYS.pageMax_ * PAGE_SIZE));
  kprintf (" %s available\n", kpsize((uintmax_t)kSYS.pageAvailable_ * PAGE_SIZE));
  kprintf ("\n\033[94m Greetings...\033[0m\n\n");

}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
