/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Main function related to processes.
 */
#include <kernel/scheduler.h>
#include <kernel/assembly.h>
#include <kernel/memory.h>
#include <kernel/vfs.h>
#include <kernel/info.h>
#include <kernel/streams.h>



// ---------------------------------------------------------------------------
/** Attach a new process to the system list.
 * \note {proc} must be locked
 */
static void ksch_attach_proc (kProcess_t *proc)
{
  assert (kislocked (&proc->lock_));

  klock (&kSYS.procLock_, LOCK_ATTACH_PROCESS);
  if (kSYS.allProcFrst_ == NULL) {
    kSYS.allProcFrst_ = proc;
    kSYS.allProcLast_ = proc;
  } else {
    kSYS.allProcLast_->nextAll_ = proc;
    kSYS.allProcLast_ = proc;
  }

  kunlock (&kSYS.procLock_);
}



int process_login(void* user, kInode_t* prg, kInode_t* dir, kInode_t* tty, const char* cmd)
{
  // assert (user != NULL);
  assert (prg != NULL);
  assert (dir != NULL);
  assert (tty != NULL);
  assert (cmd != NULL);

  if (!kasm_open (prg)) {
    __seterrno (ENOEXEC);
    return -1;
  }

  // FIXME load the image
  kAddSpace_t* mmsp = kvma_new (4 * _Kb_);
  kasm_load (mmsp, prg);

  kProcess_t* proc = KALLOC (kProcess_t);
  proc->pid_ = kSys_NewPid();
  proc->execStart_ = ltime(NULL);
  proc->workingDir_ = dir;
  proc->parent_ = NULL;
  proc->image_ = prg;
  proc->command_ = kcopystr(cmd);
  proc->memSpace_ = mmsp;

  proc->streamCap_ = 8;
  proc->openStreams_ = (kStream_t**)kalloc (sizeof(kStream_t*) * 8);
  proc->openStreams_[0] = stream_open(tty);
  proc->openStreams_[1] = stream_open(tty);
  proc->openStreams_[2] = stream_open(tty);

  klock (&proc->lock_, LOCK_PROCESS_CREATION);
  ksch_attach_proc (proc);
  proc->threadCount_++;
  proc->threadFrst_ = ksch_new_thread (proc, 0x1000000, 0xc0ffee);
  proc->threadLast_ = proc->threadFrst_;

  kunlock (&proc->lock_);
  kprintf ("Start login program [%d - %s - root<0> ]\n", proc->pid_, prg->name_);
  return proc->pid_;


  return 1;
}


// ===========================================================================
/** Create a new process, ready to start */
int ksch_create_process (kProcess_t* parent, kInode_t* image, kInode_t* dir, const char* cmd)
{
  assert (image != NULL);
  assert (parent != NULL);

  if (!kasm_open (image)) {
    __seterrno (ENOEXEC);
    return 0;
  }

  // FIXME load the image
  kAddSpace_t* mmsp = kvma_new (4 * _Kb_);
  kasm_load (mmsp, image);

  kProcess_t* proc = KALLOC (kProcess_t);
  proc->pid_ = kSys_NewPid();
  proc->execStart_ = ltime(NULL);
  proc->workingDir_ = dir;
  proc->parent_ = parent;
  proc->image_ = image;
  proc->command_ = kcopystr(cmd);
  proc->memSpace_ = mmsp;
  atomic_inc_i32 (&parent->childrenCount_);

  proc->streamCap_ = 8;
  proc->openStreams_ = (kStream_t**)kalloc (sizeof(kStream_t*) * 8);
  // @todo we must clone the stream and check if still here !
  proc->openStreams_[0] = parent->openStreams_[0]; // (kStream_t*)1;
  proc->openStreams_[1] = parent->openStreams_[1]; // kstm_create_pipe (O_WRONLY, 4 * _Kb_);
  proc->openStreams_[2] = parent->openStreams_[2]; // proc->openStreams_[1];

  klock (&proc->lock_, LOCK_PROCESS_CREATION);
  ksch_attach_proc (proc);
  proc->threadCount_++;
  proc->threadFrst_ = ksch_new_thread (proc, 0x1000000, 0xc0ffee);
  proc->threadLast_ = proc->threadFrst_;

  kunlock (&proc->lock_);
  kprintf ("Start program [%d - %s - root<0> - /mnt/cd0/USR/BIN]\n", proc->pid_, image->name_);
  return proc->pid_;
}

// ---------------------------------------------------------------------------
/** Add a new thread to the process */
int ksch_add_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg)
{
  assert (proc != NULL);
  assert (kCPU.current_ != NULL && proc == kCPU.current_->process_);

  if (proc->flags_ & PROC_EXITED)
    return __seterrno(ENOSYS);

  klock (&proc->lock_, LOCK_PROCESS_ADD_THREAD);
  kTask_t* pick = proc->threadFrst_;
  while (pick != NULL) {
    if (pick->state_ == TASK_STATE_ZOMBIE) {
      ksch_resurect_thread (pick, entry, arg);
      kunlock (&proc->lock_);
      return __noerror();
    }
    pick = pick->nextPr_;
  }

  proc->threadCount_++;
  proc->threadLast_->nextPr_ = ksch_new_thread (proc, entry, arg);
  proc->threadLast_ = proc->threadLast_->nextPr_;
  kunlock (&proc->lock_);
  return __noerror();
}

// ---------------------------------------------------------------------------
/** Push process to the garbage collector
 *  FIXME if the process is traced, or launcher is waiting, don't erase
 *  anything. When the parent stop follow, test if process have exited, in this
 *  case release ressources.
 *  FIXME processes can refer to this one as parent. We need to clean that.
 *  NOTE the only use of process parenting is for waitPid, that can be handle
 *  differently.
 */
void ksch_destroy_process (kProcess_t* proc)
{
  proc->flags_ |= PROC_EXITED;
  // FIXME if (proc->parent_ != NULL && proc->flags_ & PROC_TRACED) { // signal() }

  klock (&proc->lock_, LOCK_PROCESS_DESTROY);
  kTask_t* task = proc->threadFrst_;
  while (task != NULL) {
    kTask_t* pick = task;
    task = task->nextPr_;
    ksch_destroy_thread (pick);
  }

  proc->threadFrst_ = NULL;
  proc->threadLast_ = NULL;

  kprintf ("Exit program [%d] \n", proc->pid_);
  // FIXME release all ressources
  // FIXME remove from process list
  // FIXME send to garbage collector
  kunlock (&proc->lock_);
}

// ---------------------------------------------------------------------------
/** Exit a process by terminate all associated threads.
 *  All threads are requested to stop, once none are running,
 *  ksch_destroy_process is called.
 */
void ksch_exit (kProcess_t* proc, int status)
{
  proc->flags_ |= PROC_EXITED;
  proc->exitStatus_ = status;
  kTask_t* task = proc->threadFrst_;
  while (task != NULL) {
    ksch_abort (task);
    task = task->nextPr_;
  }
}

