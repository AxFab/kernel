#include <tasks.h>

typedef struct kContext kContext_t;
struct kContext {
  spinlock_t    lock_;
  kThread_t**   threads_;
};

kContext_t* ctx = NULL;

/*
    Each Thread have a class. All threads with superior level have to be
    sleeping before letting other process take over.
      IDLE: Lowess priority, will execute only if processor is under 50% of use
      NORMAL: Default priority for all user-space program
      HIGH: Usefull for event loop and other mostly sleeping threads
      CRITICAL: Need to be use partialy, for immediate action only



 */

void kSch_Initialize()
{
  ctx = KALLOC(kContext_t);
  ctx->threads_ = kalloc(sizeof(kThread_t*) * TASK_CLASS_COUNT);
}


int kSch_AppendThread (kThread_t* thread, int taskClass)
{
  if (taskClass < 0 || taskClass >= TASK_CLASS_COUNT)
    return __seterrno (EINVAL);

  klock (&ctx->lock_);
  kThread_t* anchor = ctx->threads_[taskClass];
  thread->next_ = anchor->next_;
  thread->prev_ = anchor;
  anchor->next_->prev_ = thread;
  anchor->next_ = thread;
  thread->class_ = taskClass;
  thread->elapseSleep_ = 0;
  thread->elapseUser_ = 0;
  thread->elapseKernel_ = 0;
  kunlock (&ctx->lock_);
  return __noerror();
}


int kSch_SetPriority ();
int kSch_SetClass ();
int kSch_SetCpuAffinity ();
int kSch_SaveThread ();

void kSch_Ticks ()
{
  // ROUND ROBIN FOR EACH CONTAINER - NOT IMPLEMENTED YET
  // WE GOT A THREAD CIRCLE LIST PER CLASS

  // WAKE UP PROCESS
  // get ElapseSleeping
  // SAVE THREAD
  // FairElapsed = ElapseSleeping / ElapseRunning

}

