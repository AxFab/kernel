#include <kernel/scheduler.h>
#include <kernel/info.h>


// ---------------------------------------------------------------------------
void kSch_Initialize ()
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
int kSch_OnTask ()
{
  return kCPU.current_ != NULL && kCPU.current_->execOnCpu_ == kCPU.cpuNo_;
}


// ---------------------------------------------------------------------------
void kSch_WakeUp (kTask_t* task)
{
  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  task->state_ = TASK_STATE_WAITING;
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
}


// ---------------------------------------------------------------------------
/** Change the status of the current executing task and save the current registers */
void kSch_StopTask (int state, kCpuRegs_t* regs)
{
  assert (kSch_OnTask ());
  assert (kCPU.current_->state_ == TASK_STATE_EXECUTING ||
    (kCPU.current_->state_ == TASK_STATE_ABORTING && state == TASK_STATE_ZOMBIE));
  assert (state != TASK_STATE_EXECUTING && state != TASK_STATE_ABORTING);

  kTask_t* task = kCPU.current_;

  asm ("cli");
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
      kSch_DestroyProcess (task->process_);
    }
  }
}


// ---------------------------------------------------------------------------
void kSch_Abort (kTask_t* task)
{
  klock (&task->lock_, LOCK_THREAD_ABORT);
  if (task->state_ == TASK_STATE_EXECUTING) {
    task->state_ = TASK_STATE_ABORTING;

  } else if (task->state_ != TASK_STATE_ZOMBIE) {
    if (task->state_ == TASK_STATE_BLOCKED) {
      kSch_CancelEvent(task);
    }

    atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
    task->state_ = TASK_STATE_ZOMBIE;
    atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
    atomic_dec_i32 (&task->process_->runningTask_);
    if (task->process_->runningTask_ == 0) {
      kSch_DestroyProcess (task->process_);
    }
  }

  kunlock (&task->lock_);
}


