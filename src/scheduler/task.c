/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Function to handle thread status.
 */
#include <kernel/scheduler.h>
#include <kernel/async.h>
#include <kernel/info.h>


// ---------------------------------------------------------------------------
void ksch_init ()
{
  // INIT kSYS
  kSYS.ticksCountMax_ = 3;
  kSYS.schedLatency_ = 20 * 1000;
  kSYS.minTimeSlice_ = 1000;
  kSYS.loadCoef_ [0] = 0.04f;
  kSYS.loadCoef_ [1] = 0.008f;
  kSYS.loadCoef_ [2] = 0.0016f;

  // INIT kCPU
  kCPU.current_ = kSYS.allTaskFrst_;
  kCPU.ready_ = true;
  sti();
}

// ---------------------------------------------------------------------------
/** Inform if the current processor have a task assign to it. */
int ksch_ontask ()
{
  return kCPU.current_ != NULL && kCPU.current_->execOnCpu_ == kCPU.cpuNo_;
}


// ---------------------------------------------------------------------------
void ksch_wakeup (kThread_t *task)
{
  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  task->state_ = SCHED_READY;
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
}


// ---------------------------------------------------------------------------
/** Change the status of the current executing task and save the current registers */
void ksch_stop (int state, kCpuRegs_t *regs)
{
  assert (ksch_ontask ());
  assert (kCPU.current_->state_ == SCHED_RUNNING ||
          (kCPU.current_->state_ == SCHED_ABORTING && state == SCHED_ZOMBIE));
  assert (state != SCHED_RUNNING && state != SCHED_ABORTING);

  kThread_t *task = kCPU.current_;

  cli();

  if (state != SCHED_ZOMBIE)
    kCpu_Save (task, regs); // FIXME Save registers

  task->execOnCpu_ = -1;
  task->elapsedUser_ += kSYS.now_ - task->lastWakeUp_;

  if (task->state_ == SCHED_ABORTING)
    task->state_ = SCHED_RUNNING;

  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  task->state_ = state;
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);

  if (state == SCHED_ZOMBIE) {
    atomic_dec_i32 (&task->process_->runningTask_);

    if (task->process_->runningTask_ == 0) {
      destroy_process (task->process_);
    }
  }
}


// ---------------------------------------------------------------------------
void ksch_abort (kThread_t *task)
{
  cli();
  klock (&task->lock_);

  if (task->state_ == SCHED_RUNNING) {
    task->state_ = SCHED_ABORTING;

  } else if (task->state_ != SCHED_ZOMBIE) {
    if (task->state_ == SCHED_BLOCKED) {
      async_cancel_event(task);
    }

    atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
    task->state_ = SCHED_ZOMBIE;
    // sched_remove (task);
    atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
    atomic_dec_i32 (&task->process_->runningTask_);

    if (task->process_->runningTask_ == 0) {
      kunlock (&task->lock_);
      destroy_process (task->process_);
      return;
    }
  }

  kunlock (&task->lock_);
}


/** Add the task on the scheduler and mark as READY */
void sched_insert(kThread_t *thread)
{
  // assert (thread->state_ == SCHED_ZOMBIE || thread->state_ == SCHED_NONE);
  // atomic_inc_i32(&thread->process_->runningTask_);
  thread->state_ = SCHED_READY;
  ksch_insert (thread);
}

void sched_wakeup(kThread_t *thread)
{
  ksch_wakeup (thread);
}

/** Remove the task form the scheduler */
void sched_remove(kThread_t *thread)
{
  ksch_remove (thread);
}





