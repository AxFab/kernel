#include <kernel/async.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>

static anchor_t waitList = ANCHOR_INIT;

// ---------------------------------------------------------------------------
/** Wake up the task waiting for an event */
void async_wakeup (kWaiting_t* wait)
{
  if (wait->task_->state_ == TASK_STATE_BLOCKED) {
    ksch_wakeup (wait->task_);
  } else {
    assert (wait->task_->state_ != TASK_STATE_ZOMBIE);
    // @todo SEND SIGNAL
  }

  if (wait->reason_ == EV_INTERVAL) { // @todo replace by flags
    wait->timeout_ += wait->param_;
  } else {

    // @todo Copy of async_cancel_event, but kfree bring trouble
    klist_remove (&waitList, &wait->waitNd_);
    if (wait->target_ != NULL)
      klist_remove (wait->target_, &wait->targetNd_);
    // Mark to delete
    wait->task_->event_ = NULL;
  }
}

// ---------------------------------------------------------------------------
/** Check the time of registers events */
void async_ticks ()
{
  ltime_t now = ltime(NULL);
  if (kSYS.timerMin_ > now)
    return;

  if (!ktrylock (&kSYS.timerLock_)) 
    return;

  kSYS.timerMin_ = INT64_MAX;
  kWaiting_t* wait = klist_begin(&waitList, kWaiting_t, waitNd_);
  while (wait != NULL) {
    kWaiting_t* del = NULL;

    if ((ltime_t)wait->timeout_ < now) {
      async_wakeup (wait);
      // @todo should we delete or not
    } else if (wait->timeout_ < kSYS.timerMin_) {
        kSYS.timerMin_ = wait->timeout_;
    }

    wait = klist_next(wait, kWaiting_t, waitNd_);
    if (del) 
      kfree(del);
  }

  kunlock (&kSYS.timerLock_);
}




// ---------------------------------------------------------------------------
/** Register to an event
  * @note maxtime is express in micro-seconds
  */
int async_event(kTask_t* task, anchor_t* targetList, int reason, long param, long maxtime)
{
  static int auto_incr = 0;
  assert (task == kCPU.current_);
  kWaiting_t* wait = KALLOC (kWaiting_t);
  wait->handle_ = auto_incr++;

  // kprintf ("async_event -- " "register [%d], %d, with %d (%d)\n", wait->handle_, reason, param, maxtime);
  // if (task->event_) {
  //   kprintf ("async_event -- " "already here [%d] %d, with %d (%d)\n", task->event_->handle_, task->event_->reason_, task->event_->param_, task->event_->handle_);
  // }
  assert (task->event_ == NULL);
  task->event_ = wait;
  wait->task_ = task;
  wait->reason_ = reason;
  wait->param_ = param;
  wait->timeout_ = (maxtime > 0 ? ltime(NULL) + maxtime * 1000LL : INT64_MAX);

  if (wait->timeout_ < kSYS.timerMin_)
    kSYS.timerMin_ = wait->timeout_;

  klist_push_back(&waitList, &wait->waitNd_);
  if (targetList != NULL) {
    wait->target_ = targetList;
    klist_push_back(targetList, &wait->targetNd_);
  }
  
  return wait->handle_;
}


// ---------------------------------------------------------------------------
/** Cancel an event */
void async_cancel_event (kTask_t* task)
{
  assert (kislocked (&task->lock_));
  kWaiting_t* wait = task->event_;
  assert (wait != NULL);
  klist_remove (&waitList, &wait->waitNd_);
  if (wait->target_ != NULL)
    klist_remove (wait->target_, &wait->targetNd_);
  kfree(wait);
  task->event_ = NULL;
}

// ---------------------------------------------------------------------------
void async_trigger (anchor_t* targetList, int reason, long param)
{
  kWaiting_t* wait = klist_begin(targetList, kWaiting_t, targetNd_);

  // kprintf ("EVT triggered  %d \n", targetList->count_);
  while (wait != NULL) {
    kWaiting_t* del = NULL;

    if (wait->reason_ == reason) {

      kprintf ("TRIGGER AN EVENT [%d] TASK %d \n", wait->reason_, wait->task_->tid_);
      async_wakeup (wait);
      // @todo should we delete or not
    }

    wait = klist_next(wait, kWaiting_t, targetNd_);
    if (del) 
      kfree(del);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
