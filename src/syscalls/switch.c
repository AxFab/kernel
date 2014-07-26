/*
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

  kInode_t* ino;
  switch (regs->eax) {
    case 0x10:
      kSch_ExitProcess (kCPU.current_->process_, regs->ecx);
      kSch_StopTask (TASK_STATE_ZOMBIE, regs);
      break;

    case 0x11:
      ino = kFs_LookFor ((char*)regs->ecx, kCPU.current_->process_->workingDir_);
      kSch_NewProcess (kCPU.current_->process_, ino, kCPU.current_->process_->workingDir_);
      break;

    case 0x23: // WRITE
      kprintf ("  %d] %s", kCPU.current_->process_->pid_, regs->edx);
      break;

    default:
      kprintf ("SYSCALL] [#%d] Unknown %d \n", kCPU.current_->tid_, regs->eax);
      kCpu_DisplayRegs (regs);

      for (;;);
      break;
  }
}



// ssize_t read (int fd, size_t length, void* buf);

// ssize_t read_sy (int sy, int fd, size_t length, void* buf);
// #define read(fd,lg,buf)  read_sy(0x22, fd, length, buf);

// ssize_t read (int fd, size_t length, void* buf)
// {
//   return read_sy (0x22, fd, length, buf);
// }


