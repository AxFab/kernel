#include <smkos/kernel.h>

void sys_ex (int no, int data)
{
  kpanic("Cpu exception '%x' %x\n", no, data);
}

void sys_irq (int no)
{
  kpanic("IRQ no '%d'\n", no);
}

