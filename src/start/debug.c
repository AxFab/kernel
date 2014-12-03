#include <kernel/core.h>
#include <kernel/cpu.h>
#include <kernel/memory.h>
#include <kernel/scheduler.h>

void kregisters (kCpuRegs_t* regs)
{
  kprintf ("dump registers at %8X\n", regs);
  kprintf ("  eax:%8x  ebx:%8x  ecx:%8x  edx:%8x    eflags:%8x\n",
    regs->eax, regs->ebx, regs->ecx, regs->edx, regs->eflags);
  kprintf ("  esi:%8x  edi:%8x  esp:%8x  ebp:%8x       eip:%8x\n",
    regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
  kprintf ("   cs:%4x  ds:%4x  es:%4x  fs:%4x  gs:%4x  ss:%4x\n",
    regs->cs, regs->ds, regs->es, regs->fs, regs->gs, regs->ss);
  kprintf ("   esp:%8x   espx:%8x   eflags:%8x\n", regs->esp, regs->espx, regs->eflags);
}

void kval (uint32_t value)
{
  kprintf ("kvalue '0x%8x'\n", value);
}

void kswitchdump (int ebp) 
{
  int* stack = &ebp; 
  kprintf ("   gs: %4x\n", stack[0]);
  kprintf ("   fs: %4x\n", stack[1]);
  kprintf ("   es: %4x\n", stack[2]);
  kprintf ("   ds: %4x\n", stack[3]);
  kprintf ("  edi: %8x\n", stack[4]);
  kprintf ("  esi: %8x\n", stack[5]);
  kprintf ("  ebp: %8x\n", stack[6]);
  kprintf ("  esp: %8x\n", stack[7]);
  kprintf ("  ebx: %8x\n", stack[8]);
  kprintf ("  edx: %8x\n", stack[9]);
  kprintf ("  ecx: %8x\n", stack[10]);
  kprintf ("  eax: %8x\n", stack[11]);
  kprintf ("  eip: %8x\n", stack[12]);
  kprintf ("   cs: %4x\n", stack[13]);
  kprintf (" flgs: %8x\n", stack[14]);
  kprintf ("  esp: %8x\n", stack[15]);
  kprintf ("   ss: %8x\n", stack[16]);

  // kvma_display (&kCPU.current_->process_->memSpace_);
  // kdump ((void*)stack[7], 32);

  // for (;;);
}