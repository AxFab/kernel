#include <scheduler.h>
#include <kinfo.h>


// ===========================================================================
int kEvt_RegSleep (kTask_t* task)
{
  task->eventParam_ += ltime(NULL);
  assert (task == kCPU.current_);
  klock (&kSYS.timerLock_, LOCK_TIMER_REGISTER_SLEEP);

  if ((ltime_t)task->eventParam_ < kSYS.timerMin_)
    kSYS.timerMin_ = (ltime_t)task->eventParam_;

  task->prevEv_ = kSYS.timerLast_;
  task->nextEv_ = NULL;
  if (kSYS.timerFrst_ == NULL) {
    kSYS.timerFrst_ = task;
    kSYS.timerLast_ = task;
  } else {
    kSYS.timerLast_->nextEv_ = task;
    kSYS.timerLast_ = task;
  }

  kunlock (&kSYS.timerLock_);
  return __noerror();
}


// ---------------------------------------------------------------------------
static int kEvt_RemoveSleep (kTask_t* task)
{
  if (task->prevEv_ == NULL)
    kSYS.timerFrst_ = task->nextEv_;
  else
    task->prevEv_->nextEv_ = task->nextEv_;

  if (task->nextEv_ == NULL)
    kSYS.timerFrst_ = task->prevEv_;
  else
    task->nextEv_->prevEv_ = task->prevEv_;

  task->prevEv_ = NULL;
  task->nextEv_ = NULL;
  task->eventParam_ = 0;
  task->eventType_ = 0;
  return __noerror();
}


// ---------------------------------------------------------------------------
int kEvt_CancelSleep (kTask_t* task)
{
  klock (&kSYS.timerLock_, LOCK_TIMER_CANCEL_SLEEP);
  kEvt_RemoveSleep (task);
  kunlock (&kSYS.timerLock_);
  return __noerror();
}


// ---------------------------------------------------------------------------
void kEvt_TimeIsUp()
{
  ltime_t now = ltime(NULL);
  if (kSYS.timerMin_ > now)
    return;

  if (!ktrylock (&kSYS.timerLock_, LOCK_TIMER_UP))
    return;

  kSYS.timerMin_ = INT64_MAX;
  kTask_t* task = kSYS.timerFrst_;
  while (task != NULL) {
    kTask_t* next = task->nextEv_;
    if ((ltime_t)task->eventParam_ < now) {

      // wake up the task
      if (task->eventType_ == TASK_EVENT_SLEEP) {
        kSch_WakeUp (task);
        kEvt_RemoveSleep (task);
      }

    } else if ((ltime_t)task->eventParam_ < kSYS.timerMin_) {
      kSYS.timerMin_ = task->eventParam_;
    }

    task = next;
  }

  kunlock (&kSYS.timerLock_);
}

