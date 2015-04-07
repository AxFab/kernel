#include <kernel/core.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>

const char *masterPaths[] = {
  "sbin/master.xe",
  "bin/master.xe",
  "master.xe",
  NULL
};

void kernel_ready ()
{

}


void kernel_start ()
{
  // assert (1); // kalloc is available, memory is virtual, screen is OK, timer is set


  // // I. Set kalloc !
  // meminit_r(&kSYS.kheap, (void*)0xD0000000, 0x20000000);


  // // II. Map the main screen
  // if (screen._mode == 1) {
  //   screen._ptr = (uint32_t*)(4 * _Mb_);
  // }


  // // III. Set timer and datetime
  // char dateStr[30];
  // struct tm dateTime;
  // RTC_GetTime (&dateTime); // todo - architecture dependant
  // asctime_r (&dateTime, dateStr);
  // kprintf ("Date: %s\n", dateStr);
  // kprintf ("Kernel Smoke v0.0 Build 0 compiled " _DATE_ "from " _OS_FULLNAME_ "\n");


  // // IV. Initialize the VFS
  // initialize_vfs ();


  // // V. Initialize device driver and use plug & play
  // VBA_Initialize (kSYS.devNd_);
  // ATA_Initialize (kSYS.devNd_);
  // kInode_t* cd = search_inode ("/dev/sdA", NULL);
  // kInode_t* mnt = search_inode ("/", NULL);
  // klock (&cd->lock_);
  // ISO_mount (cd, mnt, "usr");
  // kunlock (&cd->lock_);
  // kInode_t* sysDir = search_inode ("usr", NULL);


  // // VI. Search for debug symbols (.map)
  // kInode_t* symbIno = search_inode ("boot/kImage.map", sysDir);
  // if (symbIno != NULL)
  //   ksymbols_load(symbIno);


  // // VII. Create first terminal
  // kInode_t* screen = search_inode("/dev/fb0", NULL);
  // kInode_t* terminal = create_terminal(screen);


  // // VIII. Search for master program
  // int idx = 0;
  // kInode_t* masterIno = NULL;
  // while (masterIno == NULL) {
  //   if (masterPaths[i] == NULL)
  //     kpanic("Unable to locate the program called 'master'\n");
  //   masterIno = search_inode(masterPaths[idx], sysDir);
  //   ++idx;
  // }

  // kAssembly_t* masterImg = load_assembly(masterIno);
  // if (masterImg == NULL) {
  //   kpanic("The program '%s' can't be loaded: %s.\n",
  //     masterPaths[idx], strerror(__geterrno());
  // }


  // // VIII. Initialize master program
  // kUser_t* sysUser = create_user("system", CAP_SYSTEM);
  // login_process(masterImg, sysUser, sysDir, terminal, masterPaths[idx]);

  // // IX. Initialize per-cpu scheduler
  // kprintf ("Starting...\n");
  // ksch_init ();
}

int __delayX (int microsecond)
{

}

