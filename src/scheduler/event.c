/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      General event registration.
 */
#include <kernel/scheduler.h>
#include <kernel/stream.h>

typedef struct kEventHandler kEventHandler_t;

// struct kEventHandler
// {
//   int (*regist)(kTask_t* task);
//   int (*cancel)(kTask_t* task);
//   int (*trigger)(long param1, long param2);
// };


// // ===========================================================================
// int kevt_sleep (kTask_t* task) ;
// int kevt_timer_cancel (kTask_t* task);

// kEventHandler_t EH[] = {
//   { NULL, NULL, NULL },
//   { kevt_sleep, kevt_timer_cancel, NULL },
//   { stream_wait_regist, stream_wait_cancel, stream_wait_trigger },
// };


// // ===========================================================================
// void kevt_wait(kTask_t* task, int event, long param, kCpuRegs_t* regs)
// {
//   assert (event > 0 && event < TASK_EVENT_COUNT);
//   klock (&task->lock_, LOCK_EVENT_REGISTER);  // FIXME longest Lock detected
//   task->eventType_ = event;
//   task->eventParam_ = param;
//   ksch_stop(TASK_STATE_BLOCKED, regs);

//   EH[task->eventType_].regist (task);

//   kunlock (&task->lock_);
// }


// // ---------------------------------------------------------------------------
// void kevt_cancel (kTask_t* task)
// {
//   assert (kislocked (&task->lock_));
//   assert (task->eventType_ > 0 && task->eventType_ < TASK_EVENT_COUNT);
//   EH[task->eventType_].cancel (task);
// }


