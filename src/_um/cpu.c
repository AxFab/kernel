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


void x86_IRQ_handler(int no, void (*handler)())
{
  __unused(no);
  __unused(handler);
}

/* ----------------------------------------------------------------------- */
struct tm cpu_get_clock()
{
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
  cpause();
  longjmp(cpuJmp[main_count], 1);
}


/* ----------------------------------------------------------------------- */
void cpu_save_task(kThread_t *thread)
{
  __unused(thread);
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

int main_jmp_loop();
void kernel_ready();
void kernel_start();
void kernel_sweep();
struct kSys kSYS;

int parseChar (char **rent);
char *strtok_r(char *, const char *, char **);

void cpu_sched_ticks()
{
  sched_next(kSYS.scheduler_);
}

void cpu_start_scheduler()
{
}

void cpu_wait()
{
  printf ("  %2d.%02d] is waiting...\n", kCPU.current_->process_->pid_, kCPU.current_->paramEntry_ >> 24);
  sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
  main_jmp_loop();
}

void log_sys(const char *sbuf)
{
  printf ("  %2d.%02d] %s\n", kCPU.current_->process_->pid_, (kCPU.current_->paramEntry_ >> 24), sbuf);
}


FILE *progFp[15][15];

/* ----------------------------------------------------------------------- */
void callSys ()
{
  char buf [128];
  char *part;
  char *rent;
  size_t ent = (kCPU.current_->paramEntry_ >> 24) - 1;
  int pid = kCPU.current_->process_->pid_;

  assert (kCPU.current_ != NULL);
  part = fgets(buf, 128, progFp[pid][ent]);
  // printf ("strace -- %d, %d : %s\n", pid, ent, buf);
  assert (part != NULL);

  part = strtok_r(buf, " (,;)=", &rent);

  if (!strcmp(part, "sys_exec"))
    sys_exec_do(buf, &rent);

  else if (!strcmp(part, "sys_exit"))
    sys_exit_do(buf, &rent);

  else if (!strcmp(part, "sys_start"))
    sys_start_do(buf, &rent);

  else if (!strcmp(part, "sys_stop"))
    sys_stop_do(buf, &rent);

  else if (!strcmp(part, "sys_open"))
    sys_open_do(buf, &rent);

  else if (!strcmp(part, "sys_close"))
    sys_close_do(buf, &rent);

  else if (!strcmp(part, "sys_write"))
    sys_write_do(buf, &rent);

  else if (!strcmp(part, "sys_read"))
    sys_read_do(buf, &rent);

  else if (!strcmp(part, "sys_wait"))
    sys_wait_do(buf, &rent);

  else if (!strcmp(part, "sys_mmap"))
    sys_mmap_do(buf, &rent);

  else if (!memcmp(part, "WAKEUP", 6))
    longjmp(cpuJmp[main_count--], 2);

  else
    printf ("Unknow syscall : '%s'\n", part);

  assert (kCPU.lockCounter_ == 0); /* No throw when locked. */
  longjmp(cpuJmp[main_count], 5);
}


/* ----------------------------------------------------------------------- */
void callHdw (char *buf)
{
  int iVal;
  char *part;
  char *rent;

  part = strtok_r(buf, " (,;)=", &rent);
  assert (part != NULL);
  part = strtok_r(NULL, " (,;)=", &rent);

  if (!memcmp(part, "KEY_PRESS", 9)) {
    iVal = parseChar(&rent);
    fs_event(devKeyBoard->ino_, EV_KEYDW, iVal);
    fs_event(devKeyBoard->ino_, EV_KEYUP, iVal);
    printf("  ..] KEY %x (%c)\n", iVal, iVal);
  } else
    assert(0);

  assert (kCPU.lockCounter_ == 0); /* No throw when locked. */
  if (kCPU.current_)
    longjmp(cpuJmp[main_count], 5);
  else
    longjmp(cpuJmp[main_count], 1);
}


/* ----------------------------------------------------------------------- */
int main_jmp_loop()
{
  char buf[128];
  char *part;
  int idx;
  ++main_count;
  idx = setjmp(cpuJmp[main_count]);
  /*  0 -- Start or WakeUp
    * 1 -- Halt
    * 2 -- WakeUp Jmp
    * 3 -- Blocked
    *  5 -- OnTask
    */

  assert (kCPU.lockCounter_ == 0); /* No throw when locked. */

  if (idx == 5)
    assert (kCPU.current_ != NULL);

  if (idx == 2) /* WAKEUP */
    return 0;

  if (idx == 3) { /* BLOCKED */
    sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
    sched_next(kSYS.scheduler_);
  }

  for (;;) {
    part = fgets(buf, 128, progFp[0][0]);

    if (part == NULL) {
      printf("END OF Hdw FILE\n");
      return 0;
    } else if (buf[0] == '#' || buf[0] == ' ' || buf[0] == '\0' || buf[0] == '\n')
      continue;

    break;
  }

  switch (buf[0]) {
  case 'S':
    cpu_sched_ticks();
    break;

  case 'C':
    assert (idx != 1);
    /* assert (kCPU.state_ == KST_USERSP); */
    assert (kCPU.current_ != NULL);
    callSys ();
    break;

  case 'H':
    callHdw (buf);
    break;

  case 'T':
    assert (main_count == 0);
    kernel_sweep();
    return 0;

  case 't':
    assert (main_count == buf[1] - '0');
    kernel_sweep();
    longjmp(cpuJmp[0], 2);
  }

  assert (0);
  return -1;
}


/* ----------------------------------------------------------------------- */
int testCase (const char *dir)
{
  int idx = 0;
  int th;
  int ret;
  char tmp[120];

  main_count = -1;

  for (idx = 0; idx < 15; ++idx)
    for (th = 0; th < 15; ++th)
      progFp[idx][th] = NULL;

  /* Start threads for CPUs at { kernel_ready(); main_jmp_loop(); }
  // @todo assert -- kalloc is available, memory is virtual, screen is OK, timer is set */
  kernel_start();

  /* DEBUG ONLY */
  display_inodes();
  kprintf("\n");

  snprintf(tmp, 120, SD_DIR "/%s/Hdw.sta", dir);
  progFp[0][0] = fopen(tmp, "r");

  idx = 0;

  while (progFp[idx++][0]) {
    th = 0;

    do {
      snprintf(tmp, 120, SD_DIR "/%s/Proc%d-Th%d.strace", dir, idx, th + 1);
      progFp[idx][th++] = fopen(tmp, "r");
    } while (progFp[idx][th - 1]);
  }

  ret = main_jmp_loop();

  --idx;

  while (--idx) {
    th = 0;

    while (progFp[idx][th]) {
      fclose(progFp[idx][th++]);
    }
  }

  fclose(progFp[0][0]);
  return ret;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
