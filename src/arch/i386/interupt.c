#include <kernel/core.h>
#include <kernel/memory.h>
#include <kernel/task.h>

void kregisters (kCpuRegs_t* regs);

#include <kernel/mmu.h>


        #include <kernel/term.h>
        #include <kernel/async.h>
        kStream_t* keyboard_tty;
        /** IRQ.1 - Keyboard */
        void KBD_irq() 
        {
          unsigned char key;
          while((inb(0x64) & 0x01) == 0);
          key = inb(0x60);

          kEvent_t ev;
          ev.type_ = (key < 0x80) ? EV_KEYDW : EV_KEYUP;
          ev.keyboard_.key_ = key & 0x7F;

          if (keyboard_tty != NULL)
            term_event (keyboard_tty, &ev);
          return;
        }





/**
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
void sys_irq (int no, void* stack) 
{
  if (no == 0) {
    kSYS.now_ += CLOCK_PREC / CLOCK_HZ;
    ksch_ticks((kCpuRegs_t*)stack);
    return;
  } 

  if (no == 1) {
    KBD_irq ();
    return;
  }

  kprintf ("IRQ <#%d> \n", no);
  kdump (stack, 96);
  for (;;);
}


void sys_page_fault (size_t address, int reason, void* stack) 
{
  if (page_fault(address, reason)) 
    kpanic ("Unhandle page fault <0x%x - %x - Tsk %d> \n", address, reason, kCPU.current_->taskId_);
}


/** Hardware Exception:
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
*/
void sys_exception (int no, void* stack)
{
  if (no == 0x0D) { // General protection fault

    kprintf ("General protection fault <Tsk %d> \n", kCPU.current_->taskId_);
    kdump (stack, 96);
    kregisters(stack);
    for (;;);
  }

  kprintf ("Exception <#%x - Tsk %d> \n", no, kCPU.current_->taskId_);
  kdump (stack, 96);
  for (;;);
}

