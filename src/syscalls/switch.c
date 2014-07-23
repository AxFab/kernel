/**
 *      This file is part of the KERNEL project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Redirect system calls to the correct function.
 */
#include <inodes.h>
#include <memory.h>
#include <scheduler.h>
#include <kinfo.h>

void kCore_Syscall(kCpuRegs_t* regs)
{
  asm ("cli");

  switch (regs->eax) {
    case 0x23: // WRITE
      kprintf ("-- %s", regs->edx);
      break;

    default:
      kprintf ("SYSCALL] Unknown %d \n", regs->eax);
      kCpu_DisplayRegs (regs);
      for (;;);
      break;
  }
}
