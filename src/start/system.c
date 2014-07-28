#include "tty.h"
#include "cpu.h"
#include <kinfo.h>

#include "inodes.h"
#include "memory.h"
#include "scheduler.h"
#include <assembly.h>

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
  kFs_Initialize ();
  kVma_Initialize ();

  // Mount the system disc ----
  kInode_t* cd = kFs_LookFor ("/dev/sdA", NULL);
  kInode_t* mnt = kFs_LookFor ("/mnt/", NULL);
  ISO_Mount (cd, mnt);
  VBA_Initialize (kSYS.devNd_);

  // KRP_Mount (NULL, mnt);

  kInode_t* path = kFs_LookFor ("/mnt/OS_CORE/USR/BIN/", NULL);
  kInode_t* master = kFs_LookFor ("MASTER.", path);
  kInode_t* deamon = kFs_LookFor ("DEAMON.", path);

  // kFs_PrintAll ();


  kSch_NewProcess (NULL, master, path);
  kSch_NewProcess (NULL, deamon, path);

  kSch_Initialize ();
  // kSch_PrintTask ();

  // for (;;);
  return 0;
}