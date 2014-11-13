#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <kernel/core.h>
#include <kernel/info.h>
#include <kernel/inodes.h>
#include <kernel/scheduler.h>


int kpanic (const char *str, ...)
{
  va_list va;
  va_start (va, str);
  vprintf (str, va);
  // kstacktrace (5);
  va_end (va);
  exit (-1);
}


/**
 * @brief Clear a suite of bits on a single byte
 */
int bclearbyte (uint8_t* byte, int off, int lg)
{
  uint8_t v = byte[0];
  int mask = (0xFF << off) & 0xFF;
  if (lg + off < 8) {
    mask = (mask & ~(0xFF << (off + lg))) & 0xFF;
  }

  byte[0] = v & ~mask;
  return (v ^ mask) & mask;
}

/**
 * @brief Unset a suite of bits on a byte map
 */
int bclearbytes (uint8_t* table, int offset, int length)
{
  int ox = offset / 8;
  int oy = offset % 8;
  int r = 0;
  if (oy != 0 || length < 8) {
    if (length + oy < 8) {
      r |= bclearbyte(&table[ox], oy, length);
      length = 0;
    } else {
      r |= bclearbyte(&table[ox], oy, 8 - oy);
      length -= 8 - oy;
    }
    ox++;
  }

  while (length >= 8) {
    r |= ~table[ox];
    table[ox] = 0;
    ox++;
    length -= 8;
  }

  if (length > 0) {
    r |= bclearbyte(&table[ox], 0, length);
  }
  return r;
}

kCpuRegs_t cpu_regs;

void kCpu_Reset (kCpuRegs_t* regs, uintptr_t entry, uintmax_t param, uintptr_t stack)
{
  regs->eip = 0;
  regs->esp = regs->espx = stack;
  regs->cs = 0x23;
}

void kCpu_Switch (kCpuRegs_t* regs, uint32_t* dir, uint32_t kstack)
{
  cpu_regs.eip = regs->eip;
  cpu_regs.cs = regs->cs;
}

void kCpu_Save (kTask_t* task, kCpuRegs_t* regs)
{
  memcpy (&task->regs_, regs, sizeof(kCpuRegs_t));
  if (task->regs_.cs == 0x08) {
    task->regs_.esp = task->regs_.espx + 12;
    task->regs_.ss = 0x18;
  }
}

enum {
  UT_LEAVE,       // End of simulation
  UT_TICK,        // Simulate a clock ticks
  UT_PF_EIP,      // Simulate a page fault at EIP
  UT_PF_USTACK,   // Simulate a page fault on user stack (ESP)
  UT_PF_KSTACK,   // Simulate a page fault on kernel stack (EBP)
  UT_PF_ESI,      // Simulate a page fault while reading (ESI)
  UT_PF_EDI,      // Simulate a page fault while writing (EDI)
  UT_EXCP,        // Simulate a processor exception
  UT_IRQ,         // Simulate a hardwrare interrupt
  UT_SYSCALL,     // Simulate a syscall
};

typedef struct gInterupt gInterupt_t;
struct gInterupt {
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t call;
};

gInterupt_t masterI[] = {
  // { 0x23, 0, (uint32_t)"Hello", 5, 1 }, // PAGE FAULT ON EIP
  // { 0x23, 0, (uint32_t)"Hello", 5, 1 }, // PAGE FAULT ON USTACK
  { 0x23, 1, (uint32_t)"Hello", 5, UT_SYSCALL },
  { 0x90, 1, 2, 3, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0x11, "USR/BIN/DEAMON.", "deamon", 0, UT_SYSCALL },
  { 0x21, "/master.log", 0x41, 0, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0x23, 3, (uint32_t)"Trace] ... \n", 12, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0x23, 3, (uint32_t)"Trace] ... \n", 12, UT_SYSCALL },
  { 0x11, "BOOT/KIMAGE.", "image", 0, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_LEAVE },
};

gInterupt_t deamonI[] = {
  { 0x23, 1, (uint32_t)"Hello", 5, UT_SYSCALL },
  { 0x23, 2, (uint32_t)"Error", 5, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0x23, 2, (uint32_t)"Error", 5, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0x11, "USR/BIN/DEAMON.", "deamon1", 0, UT_SYSCALL },
  { 0x10, 0, 0, 0, UT_SYSCALL },
};

gInterupt_t deamonI1[] = {
  { 0, 0, 0, 0, UT_TICK },
  { 0x23, 1, (uint32_t)"Hello", 5, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0x21, "/master.log", 0xc2, 0, UT_SYSCALL },
  { 0x23, 2, (uint32_t)"Error", 5, UT_SYSCALL },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
  { 0, 0, 0, 0, UT_TICK },
};



kTty_t screen;

const char* sysvolume_name = NULL;
void ksysvolume ()
{
  kInode_t* disc = search_inode (sysvolume_name, kSYS.devNd_);
  if (0/* HDD */) {
    // MBR_init (disc, kSYS.devNd_);
    disc = search_inode (sysvolume_name, kSYS.devNd_);
    // FAT_init(disc, kSYS.rootNd_, "system");
  } else if (1/* CD */) {
    ISO_mount(disc, kSYS.rootNd_, "system");
  } else {
    kpanic ("Unable to recognize the system volume <%s>\n", sysvolume_name);
  }
}



const char* masterNames[] = {
  "sbin/master",
  "usr/sbin/master",
  "SBIN/MASTER.",
  "USR/SBIN/MASTER.",
  "USR/BIN/MASTER.",
};

void cli() {}

void simulation_loop ()
{
  gInterupt_t* interupt;
  ksch_ticks (&cpu_regs);
  for (;;) {
    char* program = kCPU.current_->process_->command_;
    printf ("ON %s, %d \n", program, cpu_regs.eip);

    if (!strcmp (program, "master")) {
      interupt = &masterI[cpu_regs.eip];
    } else if (!strcmp (program, "deamon")) {
      interupt = &deamonI[cpu_regs.eip];
    } else if (!strcmp (program, "deamon1")) {
      interupt = &deamonI1[cpu_regs.eip];
    }

    cpu_regs.eax = interupt->eax;
    cpu_regs.ebx = interupt->ebx;
    cpu_regs.ecx = interupt->ecx;
    cpu_regs.edx = interupt->edx;
    cpu_regs.eip ++;

    switch (interupt->call) {
      case UT_SYSCALL:
        kCore_Syscall(&cpu_regs);
        break;
      case UT_TICK:
        ksch_ticks (&cpu_regs);
        continue;

      case UT_LEAVE:
        return;
    }


  }
}

// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context */
void kpg_resolve (uint32_t address, uint32_t *table, int rights, int dirRight, uint32_t page, int reset)
{
  printf("Resolve page at %x with page %x  [%x,%d,%d,%d]\n", address, page, table, rights, dirRight, reset);
  mprotect (ALIGN_DW(address, PAGE_SIZE), PAGE_SIZE, PROT_WRITE);
  if (reset)
    memset (ALIGN_DW(address, PAGE_SIZE), 0, PAGE_SIZE);
}


int main ()
{
  int i = 0, m = sizeof(masterNames) / sizeof(char*);
  kInode_t* path = NULL;
  kInode_t* master = NULL;

  // 0.     Init simulation
  kHDW.userSpaceBase_ = (size_t)valloc(128 * _Mb_);
  kHDW.userSpaceLimit_ = kHDW.userSpaceBase_ + 128 * _Mb_;
  kHDW.pageBitmapAdd_ = malloc(8 * _Kb_);
  kHDW.pageBitmapLg_ = _Kb_;

  kHDW.kernelDir_ = (uint32_t*)malloc (PAGE_SIZE);
  kHDW.kernelTbl0_ = (uint32_t*)malloc (PAGE_SIZE);
  kHDW.screenTbl_ = (uint32_t*)malloc (PAGE_SIZE);

  mprotect(kHDW.userSpaceBase_, 128 * _Mb_, PROT_NONE);



  // 1.     On real hardware, grub start
    // 1.1. Store VBA state
    // 1.2. Init klog
    // 1.3. Detect booting device
    // 1.4. Detect RAM
  kpg_ram ((uint64_t)0, (uint64_t)640 * _Kb_);
  kpg_ram ((uint64_t)1 * _Mb_, (uint64_t)_Mb_ / 2); // MAX = 16Mb
  kpg_ram ((uint64_t)2 * _Mb_ + _Kb_ * 9, (uint64_t)18 * _Kb_);
  kpg_ram ((uint64_t)2 * _Mb_ + _Kb_ * 91, (uint64_t)67 * _Kb_);
    // 1.5. Map the kernel -- kpg_init
  kpg_init ();

  // 2.     Then the hardware is initialized
    // 2.1. GDT - Global Descriptor Table
    // 2.2. TSS - Task State Segment
    // 2.3. IDT - Interupt Descriptor Table
    // 2.4  PIC - Programmable Interrupt Controler
    // 2.5. MMU - Memory Managment Unit

  // 3.     System start
    // 3.1. Initialize kernel heap and memory
    // 3.2. Read system date and set up timer frequency
    // 3.3. Initialize VFS
  kCpu_SetStatus (CPU_STATE_SYSCALL);
  kfs_init ();
    // 3.4. Initialize default drivers
  IMG_init(kSYS.devNd_);
    // 3.5. Mount the system volume
  ksysvolume ();
    // 3.6. Look for debug symbols
    // 3.7. Start Master program
  path = search_inode ("/system/", NULL);
  while (!master && i < m)
    master = search_inode (masterNames[i++], path);
  if (!master) kpanic ("Unable to find the master program\n");
  ksch_create_process (NULL, master, path, "master");
    // 3.8. Initialize Scheduler
  ksch_init ();

  simulation_loop ();
  return 0;
}

/*
    TODO

  stub kpg_resolve
  create context switch (using mprotect)

  ...

*/