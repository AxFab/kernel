#include <scheduler.h>
#include <kinfo.h>

// ---------------------------------------------------------------------------
void kSch_Ticks () 
{
  if (!kSch_OnTask()) {
    kSch_PickNext ();

  } else if (kCPU.current_->state_ == TASK_STATE_ABORTING) {
    kSch_StopTask (TASK_STATE_ZOMBIE);
    kSch_PickNext ();

  } else if (--kCPU.current_->timeSlice_ > 0) {
    assert (kCPU.current_->state_ == TASK_STATE_EXECUTING);
    kCpu_SetStatus (CPU_STATE_USER);

  } else if (kSYS.tasksCount_[TASK_STATE_WAITING] <= 0) {
    assert (kCPU.current_->state_ == TASK_STATE_EXECUTING);
    kCPU.current_->timeSlice_ = kSch_TimeSlice (kCPU.current_);

  } else {
    kSch_StopTask (TASK_STATE_WAITING);
    kSch_PickNext ();
  }
}


// ---------------------------------------------------------------------------
int kSch_TimeSlice (kTask_t* task)
{
  ltime_t elapsed = ltime(NULL) - task->execStart_;
  if (elapsed == 0) elapsed = 1;
  long weight = (21 - task->niceValue_) * 2;
  long weightElapsed = (task->elapsedUser_ * kSYS.schedLatency_) / elapsed / kSYS.cpuCount_;
  long weightProc = (weight * kSYS.schedLatency_) / kSYS.prioWeight_;
  long sliceMicro = weightProc - weightElapsed;
  if (sliceMicro < kSYS.minTimeSlice_)
      return kSYS.minTimeSlice_ / 1000;
  return (int)sliceMicro / 1000;
}      


// ---------------------------------------------------------------------------
void kSch_PickNext () 
{
  kTask_t* pick;
  assert (!kSch_OnTask());

  // FIXME call __asm__ STI
  int waiting = atomic_dec_i32 (&kSYS.tasksCount_[TASK_STATE_WAITING]);
  if (waiting >= 0) {
    
    for (pick = kCPU.current_->nextSc_; 
        pick != kCPU.current_; 
        pick = pick->nextSc_) {

      if (pick->state_ != TASK_STATE_WAITING)
        continue;

      if (!ktrylock(&pick->lock_, LOCK_TASK_NEXT))
        continue;

      if (pick->state_ != TASK_STATE_WAITING) { // FIXME unlikely()
        kunlock (&pick->lock_);
        continue;
      }

      // Run the task
      kCPU.current_ = pick;
      pick->lastWakeUp_ = ltime(NULL);
      pick->timeSlice_ = kSch_TimeSlice (pick);
      pick->state_ = TASK_STATE_EXECUTING;
      atomic_inc_i32 (&kSYS.tasksCount_[pick->state_]);
      pick->execOnCpu_ = kCPU.cpuNo_;
      kCpu_SetStatus (CPU_STATE_USER);
      // FIXME Do switch
      kunlock (&pick->lock_);
    }

    // FIXME LOG 
  }

  atomic_inc_i32 (&kSYS.tasksCount_[TASK_STATE_WAITING]);
  kCpu_SetStatus (CPU_STATE_IDLE);
  // In case the current task is on garbadge collector
  kCPU.current_ = kCPU.current_->nextSc_; 
  // FIXME check kSYS.on_ !?
  // FIXME call __asm__ HLT
}

