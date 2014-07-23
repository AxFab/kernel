#include <scheduler.h>

typedef struct kEventHandler kEventHandler_t;

struct kEventHandler
{
  int (*regist)(kTask_t* task);
  int (*cancel)(kTask_t* task);
  int (*trigger)(long param1, long param2);
};


// ===========================================================================
int kEvt_RegSleep (kTask_t* task) ;
int kEvt_CancelSleep (kTask_t* task);

kEventHandler_t EH[] = {
  { NULL, NULL, NULL },
  { kEvt_RegSleep, kEvt_CancelSleep, NULL },
};


// ===========================================================================
void kSch_WaitEvent(kTask_t* task, int event, long param, kCpuRegs_t* regs)
{
  assert (event > 0 && event < TASK_EVENT_COUNT);
  klock (&task->lock_, LOCK_EVENT_REGISTER);  // FIXME longest Lock detected
  task->eventType_ = event;
  task->eventParam_ = param;
  kSch_StopTask(TASK_STATE_BLOCKED, regs);

  EH[task->eventType_].regist (task);

  kunlock (&task->lock_);
}


// ---------------------------------------------------------------------------
void kSch_CancelEvent (kTask_t* task)
{
  assert (kislocked (&task->lock_));
  assert (task->eventType_ > 0 && task->eventType_ < TASK_EVENT_COUNT);
  EH[task->eventType_].cancel (task);
}


