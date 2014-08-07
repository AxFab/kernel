#include <kernel/core.h>
#include <kernel/cpu.h>
#include <kernel/scheduler.h>

#define i386_TssAddress  0x1000

// ---------------------------------------------------------------------------
// Assembly IRQ routines -----------------------------------------------------
extern void kcpu_Default();
extern void kcpu_Clock();
extern void kcpu_KBoard();
extern void kcpu_SysCall();
extern void kcpu_PageFault();
extern void kcpu_Protect();
extern void kcpu_Irq15();
extern void kcpu_CMOS();

extern void kcpu_Look();

extern void IRQ0_handler();
extern void IRQ1_handler();
extern void IRQ2_handler();
extern void IRQ3_handler();
extern void IRQ4_handler();
extern void IRQ5_handler();
extern void IRQ6_handler();
extern void IRQ7_handler();
extern void IRQ8_handler();
extern void IRQ9_handler();
extern void IRQ10_handler();
extern void IRQ11_handler();
extern void IRQ12_handler();
extern void IRQ13_handler();
extern void IRQ14_handler();
extern void IRQ15_handler();

extern void IntEx00_Handler();
extern void IntEx01_Handler();
extern void IntEx02_Handler();
extern void IntEx03_Handler();
extern void IntEx04_Handler();
extern void IntEx05_Handler();
extern void IntEx06_Handler();
extern void IntEx07_Handler();
extern void IntEx08_Handler();
extern void IntEx09_Handler();
extern void IntEx0a_Handler();
extern void IntEx0b_Handler();
// ===========================================================================
#define INTGATE  0x8E00     /**< used for interruptions */
#define TRAPGATE 0xEF00     /**< used for system calls */

typedef struct kGdtEntry        kGdtEntry_t;
typedef struct kIdtEntry        kIdtEntry_t;
typedef struct kTaskSs          kTaskSs_t;

struct kGdtEntry {
    uint16_t lim0_15;
    uint16_t base0_15;
    uint8_t base16_23;
    uint8_t access;
    uint8_t lim16_19 : 4;
    uint8_t other : 4;
    uint8_t base24_31;
} __attribute__ ((packed));

struct kIdtEntry {
    uint16_t offset0_15;
    uint16_t segment;
    uint16_t type;
    uint16_t offset16_31;
} __attribute__ ((packed));

struct kTaskSs {
    uint16_t    previous_task, __previous_task_unused;
    uint32_t    esp0;
    uint16_t    ss0, __ss0_unused;
    uint32_t    esp1;
    uint16_t    ss1, __ss1_unused;
    uint32_t    esp2;
    uint16_t    ss2, __ss2_unused;
    uint32_t    cr3;
    uint32_t    eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t    es, __es_unused;
    uint16_t    cs, __cs_unused;
    uint16_t    ss, __ss_unused;
    uint16_t    ds, __ds_unused;
    uint16_t    fs, __fs_unused;
    uint16_t    gs, __gs_unused;
    uint16_t    ldt_selector, __ldt_sel_unused;
    uint16_t    debug_flag, io_map;
} __attribute__ ((packed));

// ===========================================================================
/** Write an GDT entry on the table */
static void kcpuWriteGDTEntry (int number, uint32_t base, uint32_t limit,
    int access, int other)
{
    kGdtEntry_t* ptr = (kGdtEntry_t*)(0);
    ptr += number;
    ptr->lim0_15 = (limit & 0xffff);
    ptr->base0_15 = (base & 0xffff);
    ptr->base16_23 = (base & 0xff0000) >> 16;
    ptr->access = access;
    ptr->lim16_19 = (limit & 0xf0000) >> 16;
    ptr->other = (other & 0xf);
    ptr->base24_31 = (base & 0xff000000) >> 24;
}

// ---------------------------------------------------------------------------
/** Write an IDT entry on the table */
static void kcpuWriteIDTEntry (int number, int segment, uint32_t address,
    int type)
{
    kIdtEntry_t* ptr = (kIdtEntry_t*)(0x800);
    ptr += number;
    ptr->offset0_15 = (address & 0xffff);
    ptr->segment = segment;
    ptr->type = type;
    ptr->offset16_31 = (address & 0xffff0000) >> 16;
}

// ===========================================================================
/**
 * Initialize cpu structure like GDT and IDT
 */
int kCpu_Initialize (void)
{
  int i;

  kprintf ("CPU.0 initialization\n");

  // GDT - Global Descriptor Table
  kcpuWriteGDTEntry(0, 0, 0, 0, 0);                           // Empty
  kcpuWriteGDTEntry(1, 0x0, 0xfffff, 0x9B, 0x0D);             // kernel Code
  kcpuWriteGDTEntry(2, 0x0, 0xfffff, 0x93, 0x0D);             // kernel Data
  kcpuWriteGDTEntry(3, 0x0, 0x0, 0x97, 0x0D);                 // kernel Stack
  kcpuWriteGDTEntry(4, 0x0, 0xfffff, 0xff, 0x0D);             // user Code
  kcpuWriteGDTEntry(5, 0x0, 0xfffff, 0xf3, 0x0D);             // user Data
  kcpuWriteGDTEntry(6, 0x0, 0x0, 0xf7, 0x0D);                 // user Stack
  kcpuWriteGDTEntry(7, i386_TssAddress, 0x67, 0xe9, 0x00);   // TSS

  // TSS - Task State Segment
  ((kTaskSs_t*)i386_TssAddress)->debug_flag = 0x00;
  ((kTaskSs_t*)i386_TssAddress)->io_map = 0x00;
  ((kTaskSs_t*)i386_TssAddress)->esp0 = (0x7000 - 4); // TODO Kernel Stack !
  ((kTaskSs_t*)i386_TssAddress)->ss0 = 0x18;   // TODO Kernel stack segment

  // IDT - Interupt Descriptor Table
  for (i=0; i<256; ++i) {
    kcpuWriteIDTEntry(i, 0x08, (uint32_t)kcpu_Default, INTGATE);
  }

  // Hardware Exception
  kcpuWriteIDTEntry(0x00, 0x08, (uint32_t)IntEx00_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x01, 0x08, (uint32_t)IntEx01_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x02, 0x08, (uint32_t)IntEx02_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x03, 0x08, (uint32_t)IntEx03_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x04, 0x08, (uint32_t)IntEx04_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x05, 0x08, (uint32_t)IntEx05_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x06, 0x08, (uint32_t)IntEx06_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x07, 0x08, (uint32_t)IntEx07_Handler, TRAPGATE);
  kcpuWriteIDTEntry(0x0D, 0x08, (uint32_t)kcpu_Protect, TRAPGATE);
  kcpuWriteIDTEntry(0x0E, 0x08, (uint32_t)kcpu_PageFault, TRAPGATE);

  // Hardware Interrupt Request - Master
  kcpuWriteIDTEntry(0x20, 0x08, (uint32_t)IRQ0_handler, INTGATE);
  kcpuWriteIDTEntry(0x21, 0x08, (uint32_t)kcpu_KBoard, INTGATE);

  // Hardware Interrupt Request - Slave
  // kcpuWriteIDTEntry(0x70, 0x08, (uint32_t)kcpu_CMOS, INTGATE); // 0x70 Should be CMOS-RTC
  kcpuWriteIDTEntry(0x76, 0x08, (uint32_t)IRQ14_handler, INTGATE); // Primary IDE bus
  kcpuWriteIDTEntry(0x77, 0x08, (uint32_t)IRQ15_handler, INTGATE); // Secondary IDE bus

  // Syscall entry
  kcpuWriteIDTEntry(0x30, 0x08, (uint32_t)kcpu_SysCall, TRAPGATE);

  //for (i=0x74; i<0x76; ++i)
  // kcpuWriteIDTEntry(0x76, 0x08, (uint32_t)kcpu_Look, INTGATE);


  return __noerror();
}

void init_pic(void)
{
  /* Initialisation de ICW1 */
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  /* Initialisation de ICW2 */
  outb(0x21, 0x20); /* vecteur de depart = 32 */
  outb(0xA1, 0x70); /* vecteur de depart = 96 */

  /* Initialisation de ICW3 */
  outb(0x21, 0x04);
  outb(0xA1, 0x02);

  /* Initialisation de ICW4 */
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  /* masquage des interruptions */
  outb(0x21, 0x0);
  outb(0xA1, 0x0);
}


void kCpu_Reset (kCpuRegs_t* regs, uintptr_t entry, uintmax_t param, uintptr_t stack)
{
  regs->gs = 0x2B;
  regs->fs = 0x2B;
  regs->es = 0x2B;
  regs->ds = 0x2B;
  regs->espx = stack - 0x10;
  regs->eax = param;
  regs->eip = entry;
  regs->cs = 0x23;
  regs->esp = stack - 0x10;
  regs->ss = 0x33;
}

void kCpu_Save (kTask_t* task, kCpuRegs_t* regs)
{
  memcpy (&task->regs_, regs, sizeof(kCpuRegs_t));
  if (task->regs_.cs == 0x08) {
    task->regs_.esp = task->regs_.espx + 12;
    task->regs_.ss = 0x18;
  }
}

