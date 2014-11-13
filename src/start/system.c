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

  // kTty_NewTerminal (0x7000, 0x10000 - 0x7000);
}


struct sStartInfo
{
  const char*     cmd_;
  const char*     username_;
  int             output_;
  int             input_;
  int             error_;
  int             workingDir_;  ///
  int             flags_;       ///
  int             mainWindow_;  /// Give a window/tty handler for the program
}sStartInfo_t;



struct tm RTC_GetTime ();
void RTC_EnableCMOS () ;
void RTC_DisableCMOS () ;
void PIT_Initialize (uint32_t frequency);
int ATA_Initialize(kInode_t* dev);
int VBA_Initialize(kInode_t* dev);
int ISO_mount (kInode_t* dev, kInode_t* mnt, const char* name);
void ksymbols_load (kInode_t* ino);


// ---------------------------------------------------------------------------
/** Build the environment for and create the 'master' program.
  * This program is the most thrusted process on the system.
  * It should be started by ROOT only, on directory '/usr' and be attach to 
  * TTY0.
  */
int core_master (void) 
{
  // Create TTY0
  // kInode_t* fb0 = search_inode ("/dev/fb0", NULL);
  kInode_t* tty0 = term_create(4 * _Mb_, 800, 600);
  if (tty0 == NULL)
    return -1;


  // term_write(tty0, "\e[31mBonjour\e[0m", 16);

  // Search start directory
  kInode_t* dir = search_inode ("/usr/BIN/", NULL);

  // Search program
  kInode_t* msr = search_inode ("MASTER.", dir);

  // Start program
  return process_login (NULL, msr, dir, tty0, ""); 
  // The first arg must be the user!
  // cmd can be ovewrite by grub
}


// ---------------------------------------------------------------------------
int kCore_Initialize ()
{
  if (screen._mode == 1) {
    screen._ptr = (uint32_t*)(4 * _Mb_);
  }



  // I. Print kernel info
  kprintf ("Kernel Smoke v0.0 Build 0 compiled Jul 26 2014 with gcc 4.7.2\n");

  // II. Initialize system core
  kinit ();


  // III. Set Date and initiate timer
  char tmp[510];
  struct tm dt = RTC_GetTime ();

// kprintf("Dt sec: %d \n", dt.tm_sec);
// kprintf("Dt min: %d \n", dt.tm_min);
// kprintf("Dt hour: %d \n", dt.tm_hour);
// kprintf("Dt tm_mday: %d \n", dt.tm_mday);
// kprintf("Dt tm_mon: %d \n", dt.tm_mon);
// kprintf("Dt tm_year: %d \n", dt.tm_year);
// kprintf("Dt tm_wday: %d \n", dt.tm_wday);
// kprintf("Dt tm_yday: %d \n", dt.tm_yday);
// kprintf("Dt tm_isdst: %d \n", dt.tm_mday);

  asctime_r (&dt, tmp);
  kprintf ("Date: %s\n", tmp);
  // RTC_EnableCMOS ();
  PIT_Initialize(CLOCK_HZ);


  // IV. Start system build
  kCpu_SetStatus (CPU_STATE_SYSCALL);

  kfs_init ();
  kvma_init ();
  VBA_Initialize (kSYS.devNd_);


  ATA_Initialize (kSYS.devNd_);


  // V. Mount the disc system
  kInode_t* cd = search_inode ("/dev/sdA", NULL);
  kInode_t* mnt = search_inode ("/", NULL);
  klock (&cd->lock_);
  ISO_mount (cd, mnt, "usr");
  kunlock (&cd->lock_);
  // KRP_Mount (NULL, mnt);

  // VI. Look for debug symbols
  kInode_t* sym = search_inode ("/usr/BOOT/KIMAGE.MAP", NULL);
  if (sym != NULL)
    ksymbols_load(sym);
  else
    kprintf ("We can't found the file kImage.map\n");

  core_master();


  // VII. Start default programs
  // kInode_t* path = search_inode ("/usr/USR/BIN/", NULL);
  // kInode_t* master = search_inode ("MASTER.", path);
  // kInode_t* deamon = search_inode ("DEAMON.", path);
  // ksch_create_process (NULL, master, path, "");
  // ksch_create_process (NULL, deamon, path, "");

  // kfs_log_all();

  // VIII. Initialize per-cpu scheduler
  kprintf ("Starting...\n");
  ksch_init ();

  return 0;
}
