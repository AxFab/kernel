/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Main function related to thread.
 */
#include <kernel/task.h>


// ---------------------------------------------------------------------------
/**  */
kThread_t* create_thread(kProcess_t* proc, void* pointer, long param) 
{
  assert (proc != NULL);
  assert (kislocked(&proc->lock_));

  int stackFlags = VMA_STACK | VMA_READ | VMA_WRITE;
  kThread_t* thread = KALLOC(kThread_t);
  thread->taskId_ = kSYS.taskAutoInc_;
  thread->process_ = proc;

  kVma_t* vma;
  vma = vmarea_append(&proc->memSpace_, PAGE_SIZE * 2, stackFlags);
  thread->kstack_ = vma->base_;
  vma = vmarea_append(&proc->memSpace_, 1 * _Mb_, stackFlags);
  thread->ustack_ = vma->limit_;

  klist_push_back(&proc->threads_, &thread->taskNd_);
  thread->execOnCpu_ = -1;
  thread->state_ = SCHED_WAITING;
  thread->niceValue_ = 5;
  thread->execStart_ = kSYS.now_;
  thread->execPointer_ = pointer;
  thread->param_ = param;
  thread->restart_ = true;
  atomic_inc_i32(&proc->runningTask_);
  sched_insert (thread);
  return thread;
}


// ---------------------------------------------------------------------------
/**  */
static void resurect_thread (kThread_t* thread, void* pointer, long param)
{
  assert (thread->state_ == SCHED_ZOMBIE);

  thread->execStart_ = kSYS.now_;
  thread->execPointer_ = pointer;
  thread->param_ = param;
  thread->restart_ = true;
  atomic_inc_i32(&thread->process_->runningTask_);
  thread->state_ = SCHED_WAITING;
  sched_insert (thread);
  // sched_wakeup (thread);
}


// ---------------------------------------------------------------------------
/** Add a new thread to the process */
kThread_t* append_thread (kProcess_t* proc, void* pointer, long param)
{
  assert (proc != NULL);
  assert (kCPU.current_ != NULL && proc == kCPU.current_->process_);

  if (proc->flags_ & TK_REMOVED) {

    __seterrno(ENOSYS);
    return NULL;
  }

  kThread_t* thread;
  klock (&proc->lock_);
  for_each (thread, &proc->threads_, kThread_t, taskNd_) {
    if (thread->state_ == SCHED_ZOMBIE) {
      resurect_thread(thread, pointer, param);
      kunlock (&proc->lock_);
      __seterrno(0);
      return thread;
    }
  }

  thread = create_thread (proc, pointer, param);
  // klist_push_back(&proc->threads_, &thread->taskNd_);
  kunlock (&proc->lock_);
  __seterrno(0);
  return thread;
}


// ---------------------------------------------------------------------------
/**  */
void destroy_thread (kThread_t* thread)
{
  // klock (&thread->lock_);
  // kunlock (&thread->lock_);
  sched_remove (thread);
  kfree (thread);
}



// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
