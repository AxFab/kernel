
/* SCHEDULER
  * The smoke OS scheduler is a customized round-robin based algorithm.
  * The round robin is build from a circular linked list that.
  * contains all thread ready to be executed. A semaphore hold the number
  * of available thread, that each CPU may aquire.
  * Each CPU turn around the list and execute the first suitable task.
  * This algorithm in itself is really far from optimal, so we have several
  * system that intend to optimized it.
  * First, every tasks are weighted. Each task received a different quantum
  * which is equal to:
  *         quantum = latency * latencyFactor * weight / totalWeight
  * A task will gain weight if the task need all it's quantum to finish and
  * loose if IO bounded. Meaning that CPU bounded task will have large amount
  * of time to complete their task and other will have just what they need.
  * Second, if IO bounded task have shorter elapsed time to execute, they are
  * prioritary on other tasks.
  * @note To read or update schNext_ value we need to lock on schLock_.
  */

struct kScheduler {
  kThread_t        *anchor_;
  struct semaphore  taskSem_;
  atomic_t          totalWeight_;
};

/** Insert a new thread on the scheduler */
void krn_schedul_insert(kScheduler_t *sched, kThread_t *task)
{
  klock(&sched->lock_);

  if (sched->anchor_ == NULL) {
    task->schNext_ = task;
    sched->anchor_ = task;
  } else {
    klock(&sched->anchor_->schLock_);
    task->schNext_ = sched->anchor_->schNext_;
    sched->anchor_->schNext_ = task;
    kunlock(&sched->anchor_->schLock_);
    sched->anchor_ = task;
  }

  ksem_release(&sched->taskSem_, 1);
  kunlock(&sched->lock_);
  // We add a thread on the scheduler and increase totalWeight_;
}


void krn_schedul_remove()
{

}


// A scheduler class is the controler instance of a scheduler algorithm
// scheduler class can be linked, in a single direction. The next scheduler will be called only if the first one become idle.

// Cpu have to attach themself to a scehduler but they don't have to attach to the same.

// A scheduler class dispatch timeslice between schedul entities, which can be either (group) or a thread.

// SEVERAL SCHEDULER CLASS
// CPU ATTACH THEMSELF TO ONE SCHEDULER
// SCHEDULER MAY INHERIT FROM

void scheduler_ticks(kScheduler_t *sched)
{
  // Is this scheduler finish
  // We need to find the next()
  // We
}









