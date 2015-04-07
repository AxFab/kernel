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
void ksch_insert (kThread_t *task)
{
  klock(&kSYS.schedLock_, LOCK_SCHED_INSERT);
  klock(&task->lock_, LOCK_SCHED_INSERT);
  task->state_ = SCHED_READY;
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
void ksch_remove (kThread_t *task)
{
  klock(&kSYS.schedLock_, LOCK_SCHED_REMOVE);
  assert (task->state_ == SCHED_ZOMBIE);
  assert (task->execOnCpu_ == -1);
  assert (task->event_ == NULL);

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
    kThread_t *pick = kSYS.allTaskFrst_;

    while (pick->nextSc_ != task) {
      pick = pick->nextSc_;
      assert (pick != kSYS.allTaskFrst_);
    }

    if (task == kSYS.allTaskLast_)
      kSYS.allTaskLast_ = pick;

    pick->nextSc_ = task->nextSc_;
  }

  // kprintf ("Remove task #%d\n", task->taskId_);
  kunlock(&kSYS.schedLock_);
}

// ===========================================================================
/** Create a new thread, ready to execute */
kThread_t *ksch_new_thread (kProcess_t *proc, uintptr_t entry, intmax_t arg)
{
  assert (proc != NULL);
  // kVma_t vma = {0};
  int flags_ = VMA_STACK | VMA_READ | VMA_WRITE;

  // FIXME load memory
  kThread_t *task = KALLOC (kThread_t);
  task->kstack_ = vmarea_map (&proc->memSpace_, PAGE_SIZE * 2, flags_ | VMA_KERNEL)->base_;
  task->ustack_ = vmarea_map (&proc->memSpace_, 1 * _Mb_, flags_)->limit_;
  task->taskId_ = ++kSYS.taskAutoInc_;
  task->execOnCpu_ = -1;
  task->process_ = proc;
  task->state_ = SCHED_READY;
  task->niceValue_ = 5;
  task->execStart_ = kSYS.now_;
  kCpu_Reset (&task->regs_, entry, arg, task->ustack_);
  atomic_inc_i32 (&proc->runningTask_);
  ksch_insert (task);
  return task;
}

// ---------------------------------------------------------------------------
/**  */
void ksch_destroy_thread (kThread_t *task)
{
  klock (&task->lock_, LOCK_TASK_DESTROY);
  ksch_remove (task);
  task->flags_ |= TK_REMOVED;
  kunlock (&task->lock_);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
