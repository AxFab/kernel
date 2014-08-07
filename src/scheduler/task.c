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
}

// ---------------------------------------------------------------------------
/** Inform if the current processor have a task assign to it. */
int ksch_ontask ()
{
  return kCPU.current_ != NULL && kCPU.current_->execOnCpu_ == kCPU.cpuNo_;
}


// ---------------------------------------------------------------------------
void ksch_wakeup (kTask_t* task)
{
  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  task->state_ = TASK_STATE_WAITING;
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
}


// ---------------------------------------------------------------------------
/** Change the status of the current executing task and save the current registers */
void ksch_stop (int state, kCpuRegs_t* regs)
{
  assert (ksch_ontask ());
  assert (kCPU.current_->state_ == TASK_STATE_EXECUTING ||
    (kCPU.current_->state_ == TASK_STATE_ABORTING && state == TASK_STATE_ZOMBIE));
  assert (state != TASK_STATE_EXECUTING && state != TASK_STATE_ABORTING);

  kTask_t* task = kCPU.current_;

  cli();
  if (state != TASK_STATE_ZOMBIE)
    kCpu_Save (task, regs); // FIXME Save registers

  task->execOnCpu_ = -1;
  task->elapsedUser_ += ltime(NULL) - task->lastWakeUp_;
  if (task->state_ == TASK_STATE_ABORTING)
    task->state_ = TASK_STATE_EXECUTING;

  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  task->state_ = state;
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);

  if (state == TASK_STATE_ZOMBIE) {
    atomic_dec_i32 (&task->process_->runningTask_);
    if (task->process_->runningTask_ == 0) {
      ksch_destroy_process (task->process_);
    }
  }
}


// ---------------------------------------------------------------------------
void ksch_abort (kTask_t* task)
{
  klock (&task->lock_, LOCK_THREAD_ABORT);
  if (task->state_ == TASK_STATE_EXECUTING) {
    task->state_ = TASK_STATE_ABORTING;

  } else if (task->state_ != TASK_STATE_ZOMBIE) {
    if (task->state_ == TASK_STATE_BLOCKED) {
      kevt_cancel(task);
    }

    atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
    task->state_ = TASK_STATE_ZOMBIE;
    atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
    atomic_dec_i32 (&task->process_->runningTask_);
    if (task->process_->runningTask_ == 0) {
      ksch_destroy_process (task->process_);
    }
  }

  kunlock (&task->lock_);
}


