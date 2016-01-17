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
 *      Intel x86 CPU wrapper implementation.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/kstruct/task.h>

#define TSS_CPU0_ADD 0x1000
// #include <smkos/core.h>

#include "mmu.h"

void cpu_halt_(size_t, size_t);
void cpu_restart_(size_t cr3, size_t kstk, size_t entry, size_t param, size_t ustack, size_t tssAdd);
void cpu_resume_(size_t cr3, size_t kstk, size_t stk, size_t tssAdd);
size_t mmu_newdir();

/* ----------------------------------------------------------------------- */
void cpu_halt()
{
  kernel_state (KST_IDLE);
  cpu_halt_(MMU_PREALLOC_STK, TSS_CPU0_ADD);
}


// void cpu_wait()
// {
//   kprintf ("Put Thread of %d to blocked state\n");

//   sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
//   sched_next(kSYS.scheduler_);
//   assert(0);
//   /* @todo -- this is not CPU releated. */
// }

void log_sys(const char *sbuf)
{
  kprintf ("  %2d.%02d] %s\n", kCPU.current_->process_->pid_, (kCPU.current_->paramEntry_ >> 24), sbuf);
}

/* ----------------------------------------------------------------------- */
void cpu_save_task(kThread_t *thread)
{
}


/* ----------------------------------------------------------------------- */
void cpu_run_task(kThread_t *thread)
{
  size_t eip;
  assert(thread->state_ == SCHED_READY);

  klock (&thread->process_->lock_);

  if (thread->process_->pageDir_ == 0) {
    thread->process_->pageDir_ = mmu_newdir();
  }

  kCPU.current_->state_ = SCHED_EXEC;
  kCPU.lockCounter_++; /* Prevent call to `sti' */
  kunlock (&thread->process_->lock_);
  kCPU.lockCounter_--;
  kernel_state(KST_USERSP);


  if (thread->paramEntry_ != 0) {
    eip = thread->paramEntry_;
    thread->paramEntry_ = 0;
    cpu_restart_(thread->process_->pageDir_,
                 thread->kstack_->limit_ - 0x10,
                 eip,
                 thread->paramValue_,
                 thread->ustack_->limit_ - 0x10,
                 TSS_CPU0_ADD);
  } else {
    assert (thread->stackPtr_ < thread->kstack_->limit_);
    assert (thread->stackPtr_ > thread->kstack_->limit_ - 4096 * 2);
    cpu_resume_(thread->process_->pageDir_,
                thread->kstack_->limit_ - 0x10,
                thread->stackPtr_,
                TSS_CPU0_ADD);
  }

  cpu_halt();
}


struct kSys kSYS;



/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */




