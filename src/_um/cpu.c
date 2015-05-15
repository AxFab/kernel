/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Usermode CPU wrapper implementation.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/fs.h>

#include <stdlib.h>
#include <setjmp.h>

static int main_count = -1;
static jmp_buf cpuJmp[16];

void sleep(int);

void x86_IRQ_handler(int no, void (*handler)())
{
}

/* ----------------------------------------------------------------------- */
struct tm cpu_get_clock() {
  time_t now;
  struct tm *ptm;

  time(&now);
  ptm = gmtime(&now);
  return *ptm;
}

/* ----------------------------------------------------------------------- */
void cpu_halt()
{
  kernel_state (KST_IDLE);
  sleep(1);
  longjmp(cpuJmp[main_count], 1);
}


/* ----------------------------------------------------------------------- */
void cpu_save_task(kThread_t *thread)
{
}

/* ----------------------------------------------------------------------- */
void cpu_run_task(kThread_t *thread)
{
  assert(thread->state_ == SCHED_READY);

  klock (&thread->process_->lock_);

  if (thread->process_->pageDir_ == 0) {
    thread->process_->pageDir_ = mmu_newdir();
  }

  kCPU.current_->state_ = SCHED_EXEC;
  kunlock (&thread->process_->lock_);
  kernel_state(KST_USERSP);
  longjmp(cpuJmp[main_count], 5);
}


/* ----------------------------------------------------------------------- */
int cpu_no()
{
  return 0;
}

/* ----------------------------------------------------------------------- */
void initialize_smp()
{

}


/* ----------------------------------------------------------------------- */
int evt = 0;



extern kDevice_t *devKeyBoard;

#define EV_KEYUP 10
#define EV_KEYDW 11

#include <smkos/sysapi.h>
#include <stdio.h>

void kernel_ready();
void kernel_start();
void kernel_sweep();
struct kSys kSYS;

FILE* progFp[15];

int parseChar (char **rent);
char* strtok_r(char* , const char*, char **);

void sys_write_do (char* str, char **rent);
void sys_read_do (char* str, char **rent);
void sys_exec_do (char* str, char **rent);
void sys_exit_do (char* str, char **rent);

void callSys ()
{
  int iVal;
  char* buf;
  char *part;
  char *rent;
  
  buf = (char*)malloc(128);
  assert (kCPU.current_ != NULL);
  part = fgets(buf, 128, progFp[kCPU.current_->threadId_ - 1]);
  assert (part != NULL);

  part = strtok_r(buf, " (,;)=", &rent);

  if (!memcmp(part, "sys_write", 9)) {
    sys_write_do(buf, &rent);

  } else if (!memcmp(part, "sys_read", 8)) {
    sys_read_do(buf, &rent);

  } else if (!memcmp(part, "sys_exec", 8)) {
    sys_exec_do(buf, &rent);

  } else if (!memcmp(part, "sys_exit", 8)) {
    sys_exit_do(buf, &rent);

  } else if (!memcmp(part, "KEY_PRESS", 9)) {
    iVal = parseChar(&rent);
    fs_event(devKeyBoard->ino_, EV_KEYDW, iVal);
    fs_event(devKeyBoard->ino_, EV_KEYUP,iVal);
    printf("  ..] KEY %x (%c)\n", iVal, iVal);
    free(buf);
    longjmp(cpuJmp[main_count], 5);

  } else if (!memcmp(part, "WAKEUP", 6)) {
    longjmp(cpuJmp[main_count--], 2);
  }

  free(buf);
  sched_next(kSYS.scheduler_);
}

int main_jmp_loop()
{
  int idx;
  ++main_count;
  idx = setjmp(cpuJmp[main_count]);

  assert (*__lockcounter() == 0);

  switch (idx) {
  case 0:
    // Begin or WaitFor -- Should be IRQ Clock
    kernel_state (KST_KERNSP);
    sched_next(kSYS.scheduler_);
    break;

  case 1: // Cpu Halted -- Trigger I/O -- Call kernel swip -- return 0
    kernel_sweep();
    return 0;
    // Do we have event to send !!
    // kernel_state (KST_KERNSP);
    // sched_next(kSYS.scheduler_);
    // break;

  case 2: // Wake Up -- Blocked Thread are stacking up here !!
    return 0;

  case 3: // Wait
    sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
    sched_next(kSYS.scheduler_);
    break;

  case 5:
    assert (kCPU.state_ == KST_USERSP);
    assert (kCPU.current_ != NULL);
    callSys ();
    break;
  }

  return -1;
}

void cpu_sched_ticks()
{
  sched_next(kSYS.scheduler_);
}

void cpu_start_scheduler()
{
}

void cpu_wait()
{
  printf ("  %2d] is waiting...\n", kCPU.current_->threadId_);
  sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
  main_jmp_loop();
}

/* ----------------------------------------------------------------------- */
/* At this point we leave CRTK. */
int main ()
{
  // Start threads for CPUs at { kernel_ready(); main_jmp_loop(); }
  // @todo assert -- kalloc is available, memory is virtual, screen is OK, timer is set
  kernel_start();

  // DEBUG ONLY
  display_inodes();
  kprintf("\n");
  
  progFp[0] = fopen("../SD/T1.sta", "r");
  progFp[1] = fopen("../SD/T2.sta", "r");
  progFp[2] = fopen("../SD/T3.sta", "r");

  return main_jmp_loop();
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
