/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Scheduling, tasks and cpu ressources managment.
 */
#include <smkos/kapi.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/sync.h>

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


/* ----------------------------------------------------------------------- */
/** Thread a signal */
int sched_signal (int raise, size_t data)
{
  if (kCPU.current_ == NULL) {
    kpanic ("Kernel trigger an exception ; signal: %d. at %d", raise, data);
  } else {
    
  }
  return 0;
}


/* ----------------------------------------------------------------------- */
/** Insert a new thread on the scheduler */
void sched_insert(kScheduler_t *sched, kThread_t *task)
{
  klock(&sched->lock_);

  if (sched->anchor_ == NULL) {
    task->schNext_ = task;
    sched->anchor_ = task;
  } else {
    // klock(&sched->anchor_->schLock_);
    task->schNext_ = sched->anchor_->schNext_;
    sched->anchor_->schNext_ = task;
    // kunlock(&sched->anchor_->schLock_);
    sched->anchor_ = task;
  }

  semaphore_release (&sched->taskSem_, 1);
  kunlock(&sched->lock_);
  // We add a thread on the scheduler and increase totalWeight_;
}


/* ----------------------------------------------------------------------- */
void sched_remove(kScheduler_t *sched, kThread_t *thread)
{
  klock(&sched->lock_);

  assert (thread->state_ == SCHED_ZOMBIE);
  // assert (task->event_ == NULL);

  if (kCPU.current_ == thread)
    kCPU.current_ = NULL;

  if (sched->anchor_ == thread) {
    if (thread->schNext_ == thread)
      sched->anchor_ = NULL;
    else
      sched->anchor_ = thread->schNext_;

  } else {
    kThread_t *pick = sched->anchor_;

    while (pick->schNext_ != thread)
      pick = pick->schNext_;

    pick->schNext_ = thread->schNext_;
  }

  kunlock(&sched->lock_);
}


/* ----------------------------------------------------------------------- */
/** Change the status of the current executing task and save the current registers */
void sched_stop (kScheduler_t *sched, kThread_t *thread, int state)
{
  klock (&thread->process_->lock_);
  assert(thread == kCPU.current_);
  assert(thread->state_ == SCHED_EXEC ||
          (thread->state_ == SCHED_ABORT && state == SCHED_ZOMBIE));
  assert (state != SCHED_EXEC && state != SCHED_ABORT);

  if (state != SCHED_ZOMBIE)
    cpu_save_task (thread); // FIXME Save registers

  // if (thread->state_ == SCHED_ABORTING)
  //  thread->state_ = SCHED_RUNNING;

  kCPU.current_ = thread->schNext_;
  thread->state_ = state;

  if (state == SCHED_ZOMBIE) {

    sched_remove (sched, thread);
    atomic_dec (&thread->process_->runningTask_);

    // @todo And all signal have been sended...
    if (thread->process_->runningTask_ == 0) {
      destroy_process (thread->process_);
      return;
    }

  } else if (state == SCHED_READY)
    semaphore_release(&sched->taskSem_, 1);

  klock (&thread->process_->lock_);
}

/* ----------------------------------------------------------------------- */
void sched_next(kScheduler_t *sched)
{
  if (kCPU.current_ != NULL) {
    assert(kCPU.current_->state_ == SCHED_EXEC);
    cpu_save_task (kCPU.current_);
    semaphore_release (&sched->taskSem_, 1);
    kCPU.current_->state_ = SCHED_READY;
    kCPU.current_ = kCPU.current_->schNext_;

  } else {
    kCPU.current_ = sched->anchor_;
  }

  if (semaphore_tryaquire(&sched->taskSem_, 1)) {
    if (kCPU.current_ != NULL)
      cpu_run_task(kCPU.current_);
  } else {
    cpu_halt();
  }

  kprintf("I got an error here.\n");
}


