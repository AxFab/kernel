#include "tty.h"
#include "cpu.h"
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/term.h>
#include <kernel/async.h>

/**

  Hardware Exception
    0x00  Division by zero
    0x01  Debugger
    0x02  NMI
    0x03  Breakpoint
    0x04  Overflow
    0x05  Bounds
    0x06  Invalid Opcode
    0x07  Coprocessor not available
    0x08  Double fault
    0x09  Coprocessor Segment Overrun (386 or earlier only)
    0x0A  Invalid Task State Segment
    0x0B  Segment not present
    0x0C  Stack Fault
    0x0D  General protection fault
    0x0E  Page fault
    0x0F  reserved
    0x10  Math Fault
    0x11  Alignment Check
    0x12  Machine Check
    0x13  SIMD Floating-Point Exception

  Hardware Interrupt Request
    IRQ 0 (0x20) : System Clock
    IRQ 1 : Keyboard
    IRQ 2 : N/A
    IRQ 3 : Serial port (COM2/COM4)
    IRQ 4 : Serial port (COM1/COM3)
    IRQ 5 : LPT2 (sound card)
    IRQ 6 : Floppy drive
    IRQ 7 : Parallel port (LPT1)
    IRQ 8 (0x70) : Real Time Clock (CMOS)
    IRQ 9 : N/A (PCI)
    IRQ 10 : N/A
    IRQ 11 : N/A (USB)
    IRQ 12 : N/A (PS/2)
    IRQ 13 : Math Coprocessor
    IRQ 14 : Primary HDD
    IRQ 15 : Secondary HDD


*/

// --------------------------------------------------------

int kInt_Default (kCpuRegs_t* registers)
{
  kTty_Write ("Unk.IRQ!\n");
  // for (;;);
  return 0;
}

int clockCount = 0;
int ticksCount = 0;
int kInt_Clock (kCpuRegs_t* regs)
{
  /** PIT - Timers [ +/- 1.73 sec/day ]
   */
  kSYS.now_ += CLOCK_PREC / CLOCK_HZ;
  //kprintf ("== %lld ==\n", kSYS.now_);

  // FIXME check kSYS.on_ !?
  // kTty_Putc ('.');
  if (++ticksCount > CLOCK_HZ / 100) {
    ticksCount = 0;
    // kprintf ("System clock: %lld us\n", kSYS.now_);

    ksch_ticks((kCpuRegs_t*)&regs);
  }

  if (++clockCount > CLOCK_HZ) {
    clockCount = 0;
    // kprintf ("System clock: %lld us\n", kSYS.now_);
  }

  return 0;
}

kInode_t* keyboard_tty;
int kInt_KBoard (kCpuRegs_t* registers)
{
  // return 0;

  // IRQ.1 - Keyboard 

  unsigned char i;
  while((inb(0x64) & 0x01) == 0);
  i = inb(0x60);

  kEvent_t ev;
  ev.type_ = (i < 0x80) ? EV_KEYDW : EV_KEYUP;
  ev.keyboard_.key_ = i & 0x7F;
  if (keyboard_tty != NULL)
  term_event (keyboard_tty, &ev);



 //  if (i < 0x80) {
 //   kTty_KeyPress (i & 0xFF);
 //  } else {
 //    kTty_KeyUp (i & 0x7F);
 //  }

  return 0;
}

uint64_t value = 0;
int kInt_CMOS (kCpuRegs_t* registers)
{
  /** RTC - Interval Timers from 512Hz to 8096Hz
    512Hz  -> 1953125.0    ns
    1024Hz ->  976562.5    ns
    2KHz   ->  488281.25   ns
    4KHz   ->  244140.625  ns
    8KHz   ->  122070.3125 ns
   */
  value += 9765625000; // Period in pico-seconds
  // if (value > /*9765625 */ 1024) {
  //   value = 0;
  //  kTty_Putc ('-');
  // }
  outb(0x70, 0x8C); // select register C
  inb(0x71);    // just throw away contents

  return 0;
}

/*
int kInt_SysCall (kCpuRegs_t* regs)
{
  regs->eax = kCore_Syscall (regs, (int)regs->eax,
    (uintptr_t)regs->ecx,
    (uintptr_t)regs->edx,
    (uintptr_t)regs->ebx,
    (uintptr_t)regs->esi,
    (uintptr_t)regs->edi);
  regs->ebx = __geterrno();
  return 0;
}
*/

int kpg_fault (uint32_t address);

int kInt_PageFault (uint32_t address, kCpuRegs_t* registers)
{
  // kprintf ("PageFault at 0x%x\n", address);
  return kpg_fault (address);
}


int kInt_Protect (unsigned int address, kCpuRegs_t* regs)
{
  if (regs->cs == 0x08)
    kpanic ("Kernel throw general protection fault at [%x]\n", address);

  kprintf ("task (#%d) throw general protection at [%x]: abort\n",
      kCPU.current_->process_, address);
  kregisters (regs);
  ksch_exit (kCPU.current_->process_, -1);
  ksch_stop (TASK_STATE_ZOMBIE, regs);
  ksch_pick ();
  return 0;
}



int kAta_onIRQ = 0;

int kIrq_Wait (int no)
{
  kTty_Write ("Wait for Irq15: ");
  kTty_HexChar (kAta_onIRQ, 8);
  kTty_Write ("\n");

  while (!kAta_onIRQ);

  kTty_Write ("Got Irq15!\n");
  kAta_onIRQ = 0;
  return __noerror();
}

int kIrq_Do (int no, kCpuRegs_t* registers)
{
  // kTty_Write ("Irq15!\n");
  kAta_onIRQ = 1;
  return 0;
}


int kInt_Look (unsigned int address, kCpuRegs_t* registers)
{
  kTty_Write ("LOOK IRQ...\n");
  for (;;);
  return 0;
}

void IRQ14_Enter ();
void kCpu_IRQ (int irq, kCpuRegs_t* regs)
{
  switch (irq) {
    case 14:
      IRQ14_Enter();
      break;

    default:
      kprintf ("Interrupt by IRQ %d\n", irq);
      break;
  }
}




int kInt_Exception (int no, kCpuRegs_t* regs)
{
  if (regs->cs == 0x08)
    kpanic ("Kernel throw an exception [%d]\n", no);

  if (no == 3) {

    kprintf ("Breakpoint for task [%d]: continue\n", kCPU.current_->process_->pid_);
    kregisters (regs);

  } else {

    kprintf ("task throw an exception [%d]: abort\n", no);

    ksch_exit (kCPU.current_->process_, -1);
    ksch_stop (TASK_STATE_ZOMBIE, regs);
    ksch_pick ();
  }

  return 0;
}
