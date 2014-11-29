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
#include <kernel/vfs.h>
#include <kernel/stream.h>
#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/async.h>
#include <kernel/info.h>
#include <smoke/syscall.h>
#include <kernel/params.h>

typedef int (*sys_func)(kCpuRegs_t* regs, ...);

int sys_reboot(kCpuRegs_t* regs, int code);
int sys_pause(kCpuRegs_t* regs);
int sys_exec(kCpuRegs_t* regs, const char* path, void* param);
int sys_exit(kCpuRegs_t* regs, int code);

int sys_open(kCpuRegs_t* regs, const char* path, int flags, int mode);
int sys_close(kCpuRegs_t* regs, int fd);
ssize_t sys_read(kCpuRegs_t* regs, int fd, void* buf, size_t count, off_t offset);
ssize_t sys_write(kCpuRegs_t* regs, int fd, const void* buf, size_t count, off_t offset);



int sys_itimer(kCpuRegs_t* regs, int miliseconds);
int sys_sleep(kCpuRegs_t* regs, int miliseconds);
time_t sys_time(kCpuRegs_t* regs, time_t* now);
int sys_yield(kCpuRegs_t* regs);


#define _SYS(n,f)  [n] = (sys_func)f

sys_func sys_table [] = {

  _SYS(SYS_REBOOT, sys_reboot),
  _SYS(SYS_PAUSE, sys_pause),
  _SYS(SYS_EXEC, sys_exec),
  _SYS(SYS_EXIT, sys_exit),
  // START / STOP

  _SYS(SYS_OPEN, sys_open),
  // _SYS(SYS_CLOSE, sys_close),
  _SYS(SYS_READ, sys_read),
  _SYS(SYS_WRITE, sys_write),


  // SLEEP / ITIMER
  _SYS(SYS_YIELD, sys_yield),
  _SYS(SYS_TIME, sys_time),
  _SYS(SYS_SLEEP, sys_sleep),
  _SYS(SYS_ITIMER, sys_itimer),
};


int sys_reboot(kCpuRegs_t* regs, int code)
{
  __seterrno(EPERM);
  return -1;
}

int sys_pause(kCpuRegs_t* regs)
{
  // kregisters (regs);
  ksch_stop(TASK_STATE_BLOCKED, regs);
  return 0;
}

int sys_itimer(kCpuRegs_t* regs, int miliseconds) 
{
  long microseconds = miliseconds * 1000L;
  async_event(kCPU.current_, NULL, EV_INTERVAL, microseconds, microseconds);
  // @todo create an interval timer and return it's handle
  return -1;
}

int sys_sleep(kCpuRegs_t* regs, int miliseconds)
{
  long microseconds = miliseconds * 1000L;
  async_event(kCPU.current_, NULL, EV_SLEEP, 0, microseconds);
  ksch_stop(TASK_STATE_BLOCKED, regs);
  return -1;
}


time_t sys_time(kCpuRegs_t* regs, time_t* now)
{
  kprintf (LOG, "Time is: %lld\n", kSYS.now_ / CLOCK_PREC);
  return kSYS.now_ / CLOCK_PREC;
}

int sys_yield(kCpuRegs_t* regs) 
{
  ksch_stop (TASK_STATE_WAITING, regs);
  return -1;
}




int sys_exec(kCpuRegs_t* regs, const char* path, void* param)
{
  kInode_t* ino = search_inode ((char*)regs->ecx, kCPU.current_->process_->workingDir_);
  if (ino == NULL)
    return -1;  
  sStartInfo_t* sinfo = (sStartInfo_t*)param;
  ksch_create_process (kCPU.current_->process_, ino, kCPU.current_->process_->workingDir_, sinfo->cmd_);
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
  kStream_t* stm = stream_open (-1, path, flags, mode);
  return (stm != NULL) ? stm->fd_ : -1;
}

ssize_t sys_read(kCpuRegs_t* regs, int fd, void* buf, size_t count, off_t offset)
{
  size_t lg;
  kStream_t* stream = stream_get (fd, O_RDONLY);
  if (stream == NULL) 
    return -1;

  if (!PARAM_USER_BUFFER(kCPU.current_->process_->memSpace_, buf, count))
    return -1;

  if (stream->read == NULL) {
    __seterrno(ENXIO);
    return -1;
  }

  do {
    lg = stream->read (stream, buf, count);

    if (lg == 0) {
      // @todo param is the number of byte wanted, with 0 we request a '\n' character!
      async_event(kCPU.current_, &stream->ino_->evList_, EV_READ, 0, 0);
      // kprintf (LOG, "EVT READ %s - %d \n", stream->ino_->name_, stream->ino_->evList_.count_);      
      task_pause();
      lg = stream->read (stream, buf, count);
      // kprintf (LOG, "TASK JUST WAKE UP AFTER PAUSE\n");
      // kprintf (LOG, "TASK JUST WAKE UP AFTER PAUSE\n");
      // for (;;);
      // return -1;
    }

  } while (lg == 0);

  return lg;
}

ssize_t sys_write(kCpuRegs_t* regs, int fd, const void* buf, size_t count, off_t offset)
{
  // kprintf ("sys_write -- " "on FD %d, for %d\n", fd, count);
  return stream_write (fd, buf, count);
}




void kCore_Syscall(kCpuRegs_t* regs)
{
  assert (regs != NULL);

  const int max = (int)(sizeof(sys_table) / sizeof(sys_func));
  cli();

  int no = (int)regs->eax;
  if (no >= max || sys_table[no] == NULL) {
    kprintf(LOG, "Sys] Try to call syscall function number %d (max %d).\n", no, max);
    regs->eax = -1;
    regs->edx = ENOSYS;
    return;
  }

  // kprintf (LOG, "Task %d syscall %d \n", kCPU.current_->tid_, no);
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

