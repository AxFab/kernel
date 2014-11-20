#include <kernel/async.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>

static anchor_t waitList = ANCHOR_INIT;

// ---------------------------------------------------------------------------
/** Wake up the task waiting for an event */
static void async_wakeup (kWaiting_t* wait)
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
    klist_remove (&waitList, &wait->waitNd_);
    // if (!klist_isdetach (&task->targetNd_)) { HOW !? } // Use flags!
    // Mark to delete
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
  wait->handle_ = ++auto_incr;
  wait->task_ = task;
  wait->reason_ = reason;
  wait->param_ = param;
  wait->timeout_ = (maxtime > 0 ? ltime(NULL) + maxtime * 1000LL : INT64_MAX);

  if (wait->timeout_ < kSYS.timerMin_)
    kSYS.timerMin_ = wait->timeout_;

  klist_push_back(&waitList, &wait->waitNd_);
  if (targetList != NULL)
    klist_push_back(targetList, &wait->targetNd_);
  return 0;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
