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
  // KRP_Mount (NULL, mnt);

  kInode_t* master = kFs_LookFor ("/mnt/OS_CORE/USR/BIN/MASTER.", NULL);
  kInode_t* deamon = kFs_LookFor ("/mnt/OS_CORE/USR/BIN/DEAMON.", NULL);

  // kprintf ("LOAD OF %s[%x] AND %s[%x] \n", master->name_, master, deamon->name_, deamon);
  // kInode_t* master = kFs_LookFor ("/mnt/krp/master", NULL);

  // kFs_PrintAll ();

  if (kAsm_Open (master)) {
    kSch_NewProcess (NULL, master);
  }

  if (kAsm_Open (deamon)) {
    kSch_NewProcess (NULL, deamon);
  }

  kSch_Initialize ();
  // kSch_PrintTask ();

  // for (;;);
  return 0;
}