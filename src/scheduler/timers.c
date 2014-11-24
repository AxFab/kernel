/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Routines related to task timers.
 */
#include <kernel/scheduler.h>
#include <kernel/info.h>


// anchor_t timer_list = ANCHOR_INIT;
// // ===========================================================================
// /** Suppress the task form the timer list */
// static int kevt_timer_remove (kTask_t* task)
// {
//   klist_remove (&timer_list, &task->eventNd_);
//   // if (task->prevEv_ == NULL)
//   //   kSYS.timerFrst_ = task->nextEv_;
//   // else
//   //   task->prevEv_->nextEv_ = task->nextEv_;

//   // if (task->nextEv_ == NULL)
//   //   kSYS.timerFrst_ = task->prevEv_;
//   // else
//   //   task->nextEv_->prevEv_ = task->prevEv_;

//   // task->prevEv_ = NULL;
//   // task->nextEv_ = NULL;


//   task->eventParam_ = 0;
//   task->eventType_ = 0;
//   return __noerror();
// }


// // ===========================================================================
// /** Register a task (current one) for a sleeping timer.
//  *  At the end of the timer, the task is re-schedule.
//  *  FIXME this should be handle by a timer object.
//  */
// int kevt_sleep (kTask_t* task)
// {
//   task->eventParam_ = ltime(NULL) + task->eventParam_ * 1000ULL; // Nano to Micro second
//   assert (task == kCPU.current_);
//   klock (&kSYS.timerLock_, LOCK_TIMER_REGISTER_SLEEP);

//   if ((ltime_t)task->eventParam_ < kSYS.timerMin_)
//     kSYS.timerMin_ = (ltime_t)task->eventParam_;

//   klist_push_back (&timer_list, &task->eventNd_);
//   // task->prevEv_ = kSYS.timerLast_;
//   // task->nextEv_ = NULL;
//   // if (kSYS.timerFrst_ == NULL) {
//   //   kSYS.timerFrst_ = task;
//   //   kSYS.timerLast_ = task;
//   // } else {
//   //   kSYS.timerLast_->nextEv_ = task;
//   //   kSYS.timerLast_ = task;
//   // }

//   kunlock (&kSYS.timerLock_);
//   return __noerror();
// }


// // ---------------------------------------------------------------------------
// /** Cancel the timer associated with this task
//  */
// int kevt_timer_cancel (kTask_t* task)
// {
//   klock (&kSYS.timerLock_, LOCK_TIMER_CANCEL_SLEEP);
//   kevt_timer_remove (task);
//   kunlock (&kSYS.timerLock_);
//   return __noerror();
// }


// // ---------------------------------------------------------------------------
// /** The event ticks check if some timers are expired.
//  * FIXME An improvment will be to sort timers at insertion.
//  *       Indeed, the ticks function will browse all task several time.
//  *       We better to loose time at insertion but not for wakeup.
//  */
// void kevt_ticks()
// {
//   ltime_t now = ltime(NULL);
//   if (kSYS.timerMin_ > now)
//     return;

//   if (!ktrylock (&kSYS.timerLock_, LOCK_TIMER_UP))
//     return;

//   kSYS.timerMin_ = INT64_MAX;
//   kTask_t* task; // = kSYS.timerFrst_;


//     // kprintf (LOG, "ANCHOR '%x'  '%x' \n", ino->buckets_.first_, ino->buckets_.last_);
//   for (task = klist_begin(&timer_list, kTask_t, eventNd_); 
//       task != NULL; 
//       task = klist_next(task, kTask_t, eventNd_)) {

//     if ((ltime_t)task->eventParam_ < now) {

//       // wake up the task
//       if (task->eventType_ == TASK_EVENT_SLEEP) {
//         ksch_wakeup (task);
//         kevt_timer_remove (task);
//         continue;
//       }

//       //else if (task->eventType_ == TASK_EVENT_ITIMER) {
//       //   ksch_wakeup (task);
//       //   task->eventParam_ += period; 
//       // }


//     }

//     if ((ltime_t)task->eventParam_ < kSYS.timerMin_) 
//       kSYS.timerMin_ = task->eventParam_;

//   }

//   // while (task != NULL) {
//   //   kTask_t* next = task->nextEv_;
//   //   if ((ltime_t)task->eventParam_ < now) {

//   //     // wake up the task
//   //     if (task->eventType_ == TASK_EVENT_SLEEP) {
//   //       ksch_wakeup (task);
//   //       kevt_timer_remove (task);
//   //     }

//   //   } else if ((ltime_t)task->eventParam_ < kSYS.timerMin_) {
//   //     kSYS.timerMin_ = task->eventParam_;
//   //   }

//   //   task = next;
//   // }

//   kunlock (&kSYS.timerLock_);
// }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
