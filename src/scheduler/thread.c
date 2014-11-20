/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Function related to thread creation.
 */
#include <kernel/scheduler.h>
#include <kernel/memory.h>
#include <kernel/vfs.h>
#include <kernel/info.h>


// ===========================================================================
/** Add a new task on the scheduler list as a waiting task. */
static void ksch_insert (kTask_t* task)
{
  klock(&kSYS.schedLock_, LOCK_SCHED_INSERT);
  klock(&task->lock_, LOCK_SCHED_INSERT);
  task->state_ = TASK_STATE_WAITING;
  atomic_inc_i32 (&kSYS.tasksCount_[0]);
  atomic_inc_i32 (&kSYS.tasksCount_[task->state_]);
  atomic_add_i32 (&kSYS.prioWeight_, 21 - task->niceValue_);
  if (kSYS.allTaskFrst_ == NULL) {
    kSYS.allTaskFrst_ = task;
    kSYS.allTaskLast_ = task;
    task->nextSc_ = task;
  } else {
    task->nextSc_ = kSYS.allTaskFrst_;
    kSYS.allTaskLast_->nextSc_ = task;
    kSYS.allTaskLast_ = task;
  }

  kunlock(&task->lock_);
  kunlock(&kSYS.schedLock_);
}


// ---------------------------------------------------------------------------
static void ksch_remove (kTask_t* task)
{
  klock(&kSYS.schedLock_, LOCK_SCHED_REMOVE);
  assert (task->state_ == TASK_STATE_ZOMBIE);
  assert (task->execOnCpu_ == -1);
  assert (task->eventType_ == 0); // FIXME use macro
  assert (list_isdetached(&task->eventNd_));
  // assert (task->nextEv_ == NULL);
  // assert (task->prevEv_ == NULL);

  atomic_dec_i32 (&kSYS.tasksCount_[0]);
  atomic_dec_i32 (&kSYS.tasksCount_[task->state_]);
  atomic_add_i32 (&kSYS.prioWeight_, -(21 - task->niceValue_));

  assert (kSYS.tasksCount_[0] >= 0);
  assert (kSYS.tasksCount_[task->state_] >= 0);
  assert (kSYS.prioWeight_ >= 0);

  if (kSYS.allTaskFrst_ == task) {
    kSYS.allTaskLast_->nextSc_ = kSYS.allTaskFrst_->nextSc_;
    kSYS.allTaskFrst_ = kSYS.allTaskFrst_->nextSc_;
    if (kSYS.allTaskFrst_ == task) {
      kSYS.allTaskFrst_ = NULL;
      kSYS.allTaskLast_ = NULL;
      kSYS.state_ = SYS_STATE_OFF;
    }
  } else {
    kTask_t* pick = kSYS.allTaskFrst_;
    while (pick->nextSc_ != task) {
      pick = pick->nextSc_;
      assert (pick != kSYS.allTaskFrst_);
    }

    if (task == kSYS.allTaskLast_)
      kSYS.allTaskLast_ = pick;
    pick->nextSc_ = task->nextSc_;
  }

  // kprintf ("Remove task #%d\n", task->tid_);
  kunlock(&kSYS.schedLock_);
}

// ===========================================================================
/** Create a new thread, ready to execute */
kTask_t* ksch_new_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg)
{
  assert (proc != NULL);
  kVma_t ussk = { VMA_STACK | VMA_READ | VMA_WRITE, 0L, 0L, 0, 0, 0, 0 };
  ussk.limit_ = 1 * _Mb_;
kVma_t krns = { VMA_STACK | VMA_READ | VMA_WRITE, 0L, 0L, 0, 0, 0, 0 };
  krns.limit_ = PAGE_SIZE * 2;

  // FIXME load memory
  kTask_t* task = KALLOC (kTask_t);
  task->kstack_ = kvma_mmap (proc->memSpace_, &krns)->base_;
  task->ustack_ = kvma_mmap (proc->memSpace_, &ussk)->limit_;
  task->tid_ = kSys_NewPid();
  task->execOnCpu_ = -1;
  task->process_ = proc;
  task->state_ = TASK_STATE_WAITING;
  task->niceValue_ = 5;
  task->execStart_ = ltime(NULL);
  kCpu_Reset (&task->regs_, entry, arg, task->ustack_);
  atomic_inc_i32 (&proc->runningTask_);
  ksch_insert (task);
  return task;
}

// ---------------------------------------------------------------------------
/** Ressurect a zombie thread, ready to execute */
void ksch_resurect_thread (kTask_t* task, uintptr_t entry, intmax_t arg)
{
  assert (task->state_ == TASK_STATE_ZOMBIE);

  task->execStart_ = ltime(NULL);
  kCpu_Reset (&task->regs_, entry, arg, task->ustack_);
  atomic_inc_i32 (&task->process_->runningTask_);
  ksch_wakeup (task);
}

// ---------------------------------------------------------------------------
/**  */
void ksch_destroy_thread (kTask_t* task)
{
  klock (&task->lock_, LOCK_TASK_DESTROY);
  ksch_remove (task);
  task->flags_ |= TASK_REMOVED;
  kunlock (&task->lock_);
}



// ---------------------------------------------------------------------------
void ksch_print ()
{
  kProcess_t* proc = kSYS.allProcFrst_;
  while (proc) {
    kprintf ("PROC [%d] %s, \n", proc->pid_, proc->image_->name_);
    proc = proc->nextAll_;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
