#include <inodes.h>
#include <memory.h>
#include <scheduler.h>
#include <kinfo.h>

void kCore_Syscall(kCpuRegs_t* regs)
{
  asm ("cli");

  switch (regs->eax) {
    case 0x23: // WRITE
      kprintf ("SYSCALL] %s \n", regs->edx);
      break;

    default:
      kprintf ("SYSCALL] Unknown %d \n", regs->eax);
      kCpu_DisplayRegs (regs);
      for (;;);
      break;
  }
}
