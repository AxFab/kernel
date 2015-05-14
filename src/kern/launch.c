#include <smkos/kapi.h>
#include <smkos/kstruct/user.h>


void ksymbols_load (kInode_t* ino);


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
  kUser_t* user;
  kInode_t *ino;
  kInode_t *kb;
  kInode_t *fb;
  struct tm dateTime;
  
  /* Initialize kernel environment */
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
  if (!user)
    kpanic("Unable to create system user.\n");

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

  // display_inodes();
  
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
  kInode_t* fb = search_inode ("/dev/Fb0", NULL, 0);
  scavenge_area(kSYS.mspace_);
  BMP_sync (fb);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
