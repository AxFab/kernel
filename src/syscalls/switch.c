/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Redirect system calls to the correct function.
 */
#include <kernel/inodes.h>
#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>


void kCore_Syscall(kCpuRegs_t* regs)
{
  asm ("cli");

  kInode_t* ino;

  kCpu_SetStatus (CPU_STATE_SYSCALL);
  kprintf ("syscall] enter {%d} -> <%d> -- \n", kCPU.current_->process_->pid_, regs->eax);

  switch (regs->eax) {
    case 0x10: // EXIT
      kSch_ExitProcess (kCPU.current_->process_, regs->ecx);
      kSch_StopTask (TASK_STATE_ZOMBIE, regs);
      kSch_PickNext ();
      break;

    case 0x11: // EXEC
      ino = kfs_lookup ((char*)regs->ecx, kCPU.current_->process_->workingDir_);
      kSch_NewProcess (kCPU.current_->process_, ino, kCPU.current_->process_->workingDir_);
      break;

    case 0x21: // OPEN
      regs->eax = kSys_Open (-1, (char*)regs->ecx, (int)regs->edx, (int)regs->ebx);
      kprintf ("OPEN %d, %d \n", regs->eax, __geterrno());
      break;

    case 0x22: // READ
      regs->eax = kSys_Read ((int)regs->ecx, (void*)regs->edx, (size_t)regs->ebx, (off_t)-1);
      break;

    case 0x23: // WRITE
      if (regs->ecx > 2)
        regs->eax = kSys_Write ((int)regs->ecx, (void*)regs->edx, (size_t)regs->ebx, (off_t)-1);
      else
        kprintf ("  %d] %s", kCPU.current_->process_->pid_, regs->edx);

      break;

    default:
      kprintf ("SYSCALL] [#%d] Unknown %d \n", kCPU.current_->tid_, regs->eax);
      kCpu_DisplayRegs (regs);

      for (;;);
      break;
  }

  kprintf ("syscall] leave {%d} -> <%d> -- \n", kCPU.current_->process_->pid_, regs->eax);
  kCpu_SetStatus (CPU_STATE_USERMODE);
}



// ssize_t read (int fd, size_t length, void* buf);

// ssize_t read_sy (int sy, int fd, size_t length, void* buf);
// #define read(fd,lg,buf)  read_sy(0x22, fd, length, buf);

// ssize_t read (int fd, size_t length, void* buf)
// {
//   return read_sy (0x22, fd, length, buf);
// }


