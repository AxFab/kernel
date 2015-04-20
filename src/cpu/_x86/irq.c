#include <smkos/kernel.h>

/* ----------------------------------------------------------------------- */
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
void sys_ex (int no, int data)
{
  kpanic("Cpu exception '%x' %x\n", no, data);
}


void(*x86_irq_hanlder[16])();


/* ----------------------------------------------------------------------- */

/**Hardware Interrupt Request
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
void sys_irq (int no)
{
  if (no < 0 || no >= 16)
    kpanic ("IRQ no %d !?\n", no);

  if (x86_irq_hanlder[no] == NULL) {
    // kprintf ("IRQ no %d: ignored\n", no);
    return;
  }

  x86_irq_hanlder[no]();
}


/* ----------------------------------------------------------------------- */
int system_call (int no, size_t p1, size_t p2, size_t p3, size_t p4, size_t p5)
{
  kprintf("SYSCALL %d] %8x, %8x, %8x, %8x, %8x\n", no, p1, p2, p3, p4, p5);
  for (;;);
  return 0;
}


void sys_call(size_t* params)
{
  int err = system_call(params[11], params[10], params[9], params[8], params[7], params[6]);
  params[11] = err;
  params[9] = __geterrno();
}


/* ----------------------------------------------------------------------- */
void x86_IRQ_handler(int no, void (*handler)())
{
  if (no < 0 || no >= 16)
    return;

  x86_irq_hanlder[no] = handler;
}

void general_protection (size_t errcode)
{
  kdump(&errcode, 64);
  kpanic ("General Protection Fault : '0x%08x'\n", errcode);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
