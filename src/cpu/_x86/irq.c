#include <smkos/kernel.h>

void sys_ex (int no, int data)
{
  kpanic("Cpu exception '%x' %x\n", no, data);
}


void(*x86_irq_hanlder[16])();

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

void sys_call(void* params)
{
  kprintf("SYSCALL\n");
}

void x86_IRQ_handler(int no, void (*handler)())
{
  if (no < 0 || no >= 16)
    return;

  x86_irq_hanlder[no] = handler;
}

