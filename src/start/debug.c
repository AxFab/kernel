#include <kernel/core.h>
#include <kernel/cpu.h>

void kregisters (kCpuRegs_t* regs)
{
  kprintf ("  eax:%8x  ebx:%8x  ecx:%8x  edx:%8x    eflags:%8x\n",
    regs->eax, regs->ebx, regs->ecx, regs->edx, regs->eflags);
  kprintf ("  esi:%8x  edi:%8x  esp:%8x  ebp:%8x       eip:%8x\n",
    regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
  kprintf ("   cs:%4x  ds:%4x  es:%4x  fs:%4x  gs:%4x  ss:%4x\n",
    regs->cs, regs->ds, regs->es, regs->fs, regs->gs, regs->ss);
  kprintf ("   esp:%8x   espx:%8x   eflags:%8x\n", regs->esp, regs->espx, regs->eflags);
}

