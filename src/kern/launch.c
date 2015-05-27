#include <smkos/kapi.h>
#include <smkos/kstruct/user.h>
#include <smkos/kstruct/map.h>
#include <smkos/drivers.h>


void ksymbols_load (kInode_t *ino);

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
  memset (&kSYS, 0, sizeof (kSYS));
  memset (&kCPU, 0, sizeof (kCPU));
  kernel_state (KST_KERNSP);
  kprintf("SmokeOS " _VTAG_ ", build at " __DATE__ " from git:" _GITH_ " on " _OSNAME_ ".\n");
  mmu_load_env();

  /* Initialize time managment */
  dateTime = cpu_get_clock();
  kprintf ("Date: %s\n", asctime(&dateTime));

  initialize_smp();

  /* Initialize the VFS */
  initialize_vfs();

  /* Search kernel helper files */
  ino = search_inode ("boot/kImage.map", kSYS.sysIno_, 0);

  if (ino)
    ksymbols_load(ino);

  /* Create basic users */
  user = create_user("system", CAP_SYSTEM);
  assert_msg(user, "Unable to create system user.");

  /* Load master program */
  idx = 0;

  while (masterPaths[idx]) {
    ino = search_inode (masterPaths[idx], kSYS.sysIno_, 0);

    if (ino && NULL != load_assembly(ino))
      break;

    ++idx;
  }

  // Open Graphic buffer
  kb = search_inode ("/dev/Kb0", NULL, 0);
  fb = search_inode ("/dev/Fb0", NULL, 0);
  create_subsys(kb, fb);

  if (!masterPaths[idx])
    kpanic("Unable to find startup program 'MASTER'\n");

  create_logon_process(ino, user, kSYS.sysIno_, masterPaths[idx]);
  scavenge_area(kSYS.mspace_);

  kprintf ("CPU %d is ready\n", kCpuNo);
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
  mmu_leave_env();
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
