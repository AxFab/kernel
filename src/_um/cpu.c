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

static jmp_buf cpuJmp;

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
  longjmp(cpuJmp, 1);
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
  longjmp(cpuJmp, 5);
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

void kernel_ready();
void kernel_start();
void kernel_sweep();
struct kSys kSYS;

int pes = 0;
int main_jmp_loop()
{
  int ret;
  char* buf; 
  int idx = setjmp(cpuJmp);

  switch (idx) {
  case 0:
    kernel_state (KST_KERNSP);
    break;

  case 1: // Cpu Halted -- Trigger I/O -- Call kernel swip -- return 0
    if (evt > 1) {
      kernel_sweep();
      return 0;
    }

    ++evt;
    fs_event(devKeyBoard->ino_, EV_KEYDW, 0x20);
    fs_event(devKeyBoard->ino_, EV_KEYUP, 0x20);
    fs_event(devKeyBoard->ino_, EV_KEYDW, 0x20);
    fs_event(devKeyBoard->ino_, EV_KEYUP, 0x20);
    break;

    // Do we have event to send !!
    // kernel_state (KST_KERNSP);
    // sched_next(kSYS.scheduler_);
    // break;

  case 5:
    assert (kCPU.state_ == KST_USERSP);
    assert (kCPU.current_ != NULL);

    switch (pes++) {
    case 0:
      ret = sys_write(1, "Hello world!\n", 13, 0);
      break;
    case 1:
      fs_event(devKeyBoard->ino_, EV_KEYDW, 'F');
      fs_event(devKeyBoard->ino_, EV_KEYUP, 'F');
      fs_event(devKeyBoard->ino_, EV_KEYDW, 'a');
      fs_event(devKeyBoard->ino_, EV_KEYUP, 'a');
      fs_event(devKeyBoard->ino_, EV_KEYDW, 'b');
      fs_event(devKeyBoard->ino_, EV_KEYUP, 'b');
      fs_event(devKeyBoard->ino_, EV_KEYDW, '\n');
      fs_event(devKeyBoard->ino_, EV_KEYUP, '\n');
      break;
    case 2:
      ret = sys_write(1, "Login :: ", 9, -1);
      break;
    case 3:
      buf = (char*)malloc(128);
      ret = sys_read(0, buf, 128, -1);
      buf[ret] = '\0';
      printf("Read: '%s'\n", buf);
      free(buf);
      break;
    case 4:
      ret = sys_write(1, "\x1b[31mFailed\n", 12, -1);
      break;
    case 5:
      ret = sys_exit(0, 0);
      break;
    }
    break;

  default:
    return 0;
  }

  sched_next(kSYS.scheduler_);
  return -1;
}



void cpu_sched_ticks()
{
  sched_next(kSYS.scheduler_);
}

void cpu_start_scheduler()
{
}

/* ----------------------------------------------------------------------- */
/* At this point we leave CRTK. */
int main ()
{
  // Start threads for CPUs at { kernel_ready(); main_jmp_loop(); }
  kernel_start();

  // DEBUG ONLY
  display_inodes();
  kprintf("\n");

  // assert (1); // kalloc is available, memory is virtual, screen is OK, timer is set
  /// @todo Set kalloc !
  /// @todo Map the main screen
  /// @todo Create first terminal

  return main_jmp_loop();
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
