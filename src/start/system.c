#include "tty.h"
#include "cpu.h"
#include <kinfo.h>

#include "inodes.h"
#include "memory.h"
#include "scheduler.h"

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
  kSch_Initialize ();

  // Mount the system disc ---- 
  kInode_t* cd = kFs_LookFor ("/dev/sdA", NULL);
  kInode_t* mnt = kFs_LookFor ("/mnt/", NULL);
  ISO_Mount (cd, mnt);


  kInode_t* kimg = kFs_LookFor ("/mnt/OS_CORE/BOOT/KIMAGE.", NULL);

  uint8_t buff [2048];
  kFs_Read (kimg, buff, 1, 0);
  kTty_HexDump (buff, 512);

  kFs_PrintAll ();


  SIZEOF(kCpuRegs_t);
  SIZEOF(kTty_t);
  // SIZEOF(kUser_t);
  SIZEOF(kStat_t);
  SIZEOF(kInode_t);
  // SIZEOF(kFsys_t);
  SIZEOF(kDevice_t);
  SIZEOF(kResxFile_t);
  SIZEOF(kFileOp_t);
  SIZEOF(kVma_t);
  SIZEOF(kAddSpace_t);
  SIZEOF(kProcess_t);
  SIZEOF(kTask_t);
  // SIZEOF(kAssembly_t);
  // SIZEOF(kSection_t);

  return 0;
}


void kCore_Syscall()
{
  kprintf ("SYSCALL\n");
}
