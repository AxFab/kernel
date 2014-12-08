#include <kernel/core.h>
#include <kernel/task.h>
#include <smoke/syscall.h>

typedef int (*sys_func)(void* stack, ...);

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

static sys_func sys_table [] = {

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



void sys_enter (int no, void* stack)
{
  int ret;
  const int max = (int)(sizeof(sys_table) / sizeof(sys_func));
  uint32_t ebx = ((kCpuRegs_t*)stack)->ebx;
  uint32_t ecx = ((kCpuRegs_t*)stack)->ecx;
  uint32_t edx = ((kCpuRegs_t*)stack)->edx;
  uint32_t esi = ((kCpuRegs_t*)stack)->esi;
  uint32_t edi = ((kCpuRegs_t*)stack)->edi;

  cli();
  
  kprintf ("SysCall Enter <#%x - Tsk %d> \n", no, kCPU.current_->taskId_);

  if (no >= max || sys_table[no] == NULL) {
    kprintf("Sys] Try to call syscall function number %d (max %d).\n", no, max);
    return;
  }

  kCpu_SetStatus (CPU_STATE_SYSCALL);
  ret = sys_table[no](stack, ecx, edx, ebx, esi, edi);

  if (!ksch_ontask()) {
    ksch_pick ();
  } else {
    ((kCpuRegs_t*)stack)->eax = ret;
    ((kCpuRegs_t*)stack)->edx = __geterrno();
    kCpu_SetStatus (CPU_STATE_USERMODE);
  }

  kprintf ("SysCall Exit <#%x - Tsk %d> \n", no, kCPU.current_->taskId_);
}

