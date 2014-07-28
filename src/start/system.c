#include "tty.h"
#include "cpu.h"
#include <kernel/info.h>

#include "kernel/inodes.h"
#include "kernel/memory.h"
#include "kernel/scheduler.h"
#include <kernel/assembly.h>

void kinit()
{
  meminit_r(&kSYS.kheap, (void*)0xD0000000, 0x20000000);
  // kCPU.asp = kMem_Create (4 * _Mb_);
}



struct tm RTC_GetTime ();
void RTC_EnableCMOS () ;
void RTC_DisableCMOS () ;
void PIT_Initialize (uint32_t frequency);
int ATA_Initialize(kInode_t* dev);
int VBA_Initialize(kInode_t* dev);

  int ISO_Mount (kInode_t* dev, kInode_t* mnt);


int kCore_Initialize ()
{
  if (screen._mode == 1) {
    screen._ptr = (uint32_t*)(4 * _Mb_);
  }

  // kinit ();
  // kTty_NewTerminal (0x7000, 0x10000 - 0x7000);
  kTty_Update ();
  kprintf ("Kernel Smoke v0.0 Build 0 compiled Jul 26 2014 with gcc 4.7.2\n");

  char tmp[510];
  struct tm dt = RTC_GetTime ();
  asctime_r (&dt, tmp);
  kprintf ("Date: %s", tmp);

  // RTC_EnableCMOS ();
  PIT_Initialize(CLOCK_HZ);

  // - - - - - - - - - - - - - - - - - - -
  //  - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - - - - - - -

  kinit ();

  kCpu_SetStatus (CPU_STATE_SYSCALL);

  kfs_init ();
  kVma_Initialize ();

  ATA_Initialize (kSYS.devNd_);
  VBA_Initialize (kSYS.devNd_);

  // Mount the system disc ----
  kInode_t* cd = kfs_lookup ("/dev/sdA", NULL);
  kInode_t* mnt = kfs_lookup ("/mnt/", NULL);
  ISO_Mount (cd, mnt);

  // KRP_Mount (NULL, mnt);

  kInode_t* path = kfs_lookup ("/mnt/OS_CORE/USR/BIN/", NULL);
  kInode_t* master = kfs_lookup ("MASTER.", path);
  kInode_t* deamon = kfs_lookup ("DEAMON.", path);

  // kfs_log_all ();


  kSch_NewProcess (NULL, master, path);
  kSch_NewProcess (NULL, deamon, path);

  kSch_Initialize ();
  // kSch_PrintTask ();

  // for (;;);
  return 0;
}