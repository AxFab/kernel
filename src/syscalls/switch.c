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
#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>
#include <smoke/syscall.h>

typedef int (*sys_func)(kCpuRegs_t* regs, ...);

int sys_reboot(kCpuRegs_t* regs, int code);
int sys_exec(kCpuRegs_t* regs, void* param);
int sys_exit(kCpuRegs_t* regs, int code);

int sys_open(kCpuRegs_t* regs, const char* path, int flags, int mode);
int sys_close(kCpuRegs_t* regs, int fd);
ssize_t sys_read(kCpuRegs_t* regs, int fd, void* buf, size_t count, off_t offset);
ssize_t sys_write(kCpuRegs_t* regs, int fd, const void* buf, size_t count, off_t offset);



int sys_itimer(kCpuRegs_t* regs, int miliseconds);
time_t sys_time(kCpuRegs_t* regs, time_t* now);
int sys_waitobj(kCpuRegs_t* regs, int handle, int what, int flags);



#define _SYS(n,f)  [n] = (sys_func)f

sys_func sys_table [] = {

  _SYS(SYS_REBOOT, sys_reboot),
  // START / STOP
  // SLEEP / ITIMER
  // OPEN / CLOSE
  _SYS(SYS_EXEC, sys_exec),
  _SYS(SYS_EXIT, sys_exit),

  _SYS(SYS_OPEN, sys_open),
  // _SYS(SYS_CLOSE, sys_close),
  _SYS(SYS_READ, sys_read),
  _SYS(SYS_WRITE, sys_write),


  _SYS(SYS_TIME, sys_time),
  _SYS(SYS_ITIMER, sys_itimer),
  _SYS(SYS_WAIT, sys_waitobj),
};


int sys_reboot(kCpuRegs_t* regs, int code)
{
  __seterrno(EPERM);
  return -1;
}

int sys_itimer(kCpuRegs_t* regs, int miliseconds) 
{
  return -1;
}

time_t sys_time(kCpuRegs_t* regs, time_t* now)
{
  kprintf ("Time is: %lld\n", kSYS.now_);
  return kSYS.now_ * CLOCK_HZ / 1000000;
}

int sys_waitobj(kCpuRegs_t* regs, int handle, int what, int flags) 
{
  kevt_wait(kCPU.current_, TASK_EVENT_SLEEP, (1000ULL * 1000ULL * 100ULL) * 3ULL, regs);
  return -1;
}




int sys_exec(kCpuRegs_t* regs, void* param)
{
  kInode_t* ino = kfs_lookup ((char*)regs->ecx, kCPU.current_->process_->workingDir_);
  if (ino == NULL)
    return -1;  
  ksch_create_process (kCPU.current_->process_, ino, kCPU.current_->process_->workingDir_, regs->edx);
  return -1;
}

int sys_exit(kCpuRegs_t* regs, int code)
{
  ksch_exit (kCPU.current_->process_, code);
  ksch_stop (TASK_STATE_ZOMBIE, regs);
  return -1;
}



int sys_open(kCpuRegs_t* regs, const char* path, int flags, int mode)
{
  return kstm_open (-1, path, flags, mode);
}

ssize_t sys_read(kCpuRegs_t* regs, int fd, void* buf, size_t count, off_t offset)
{
  return kstm_read (fd, buf, count, (off_t)-1);
}

ssize_t sys_write(kCpuRegs_t* regs, int fd, const void* buf, size_t count, off_t offset)
{
  return kstm_write (fd, buf, count, (off_t)-1);
}




void kCore_Syscall(kCpuRegs_t* regs)
{
  const int max = (int)(sizeof(sys_table) / sizeof(sys_func));
  cli();

  int no = (int)regs->eax;
  if (no >= max || sys_table[no] == NULL) {
    kprintf("Sys] Try to call syscall function number %d (max %d).\n", no, max);
    regs->eax = -1;
    regs->edx = ENOSYS;
    return;
  }

  kCpu_SetStatus (CPU_STATE_SYSCALL);
  int ret = sys_table[no](regs, 
                (void*)regs->ecx, 
                (void*)regs->edx, 
                (void*)regs->ebx, 
                (void*)regs->esi, 
                (void*)regs->edi);

  if (!ksch_ontask()) {
    ksch_pick ();
  } else {
    regs->eax = ret;
    regs->edx = __geterrno();
    kCpu_SetStatus (CPU_STATE_USERMODE);
  }

}

