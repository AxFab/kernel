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
#include <kernel/task.h>

struct kSession
{
  kUser_t*  user_;
  int32_t   processCount_;
};


static kSession_t* alloc_session (kUser_t* user)
{
  kSession_t* session = KALLOC(kSession_t);
  session->user_ = user;
  session->processCount_ = 1;
  atomic_inc_i32(&user->processCount_);
  return session;
}

// ---------------------------------------------------------------------------
static void destroy_session (kSession_t* session) 
{
  assert (session->processCount_ == 0);
  kfree(session);
}

// ---------------------------------------------------------------------------
static kProcess_t* alloc_process (kAssembly_t* asmImg)
{
  kProcess_t* process = KALLOC (kProcess_t);
  process->pid_ = ++kSYS.pidAutoInc_;
  process->execStart_ = kSYS.now_;
  // proc->commandLine_ = kstrdup(cmd);
  process->asmImg_ = asmImg;
  addspace_init(&process->memSpace_, asmImg->flags_);

  klock (&process->lock_);
  klist_push_back(&kSYS.processes_, &process->allNd_);

  kThread_t* start = create_thread(process, asmImg->entryPoint_, 0xc0ffee);
  if (start == NULL) {
    kfree(process);
    return NULL;
  }

  klist_push_back(&process->children_, &process->siblingNd_);
  // klist_push_back(&process->threads_, &start->taskNd_);
  return process;
}


// ---------------------------------------------------------------------------
/** Create a parentless process with a new user */
kProcess_t* login_process (kAssembly_t* asmImg, kUser_t* user, kInode_t* dir, kInode_t* term, const char* cmd)
{
  assert (asmImg != NULL);
  assert (user != NULL);
  assert (dir != NULL);
  assert (term != NULL);

  kProcess_t* proc = alloc_process(asmImg);
  if (proc == NULL)
    return NULL;

  proc->workingDir_ = dir;
  proc->session_ = alloc_session (user);
  // stream_tty (proc, term);
  // if (keyboard_tty == NULL)
  //   keyboard_tty = proc->openStreams_[0];

  if (map_assembly(&proc->memSpace_, asmImg)) {
    kunlock (&proc->lock_);
    destroy_process (proc);
    return NULL;
  }
  
  kunlock (&proc->lock_);
  return proc;
}


// ---------------------------------------------------------------------------
/** Create a new process on the same session of the parent */
kProcess_t* create_process (kAssembly_t* asmImg, const char* cmd, const char* env)
{
  assert (asmImg != NULL);
  assert (kCPU.current_ != NULL);

  kProcess_t* proc = alloc_process(asmImg);
  if (proc == NULL)
    return NULL;

  proc->parent_ = kCPU.current_->process_;
  proc->workingDir_ = proc->parent_->workingDir_;
  proc->session_ = proc->parent_->session_;
  atomic_inc_i32(&proc->session_->processCount_);
  atomic_inc_i32(&proc->session_->user_->processCount_);
  // if (stream_reopen (proc->parent_, 0) ) {
  // }

  if (map_assembly(&proc->memSpace_, asmImg)) {
    kunlock (&proc->lock_);
    destroy_process (proc);
    return NULL;
  }

  kunlock (&proc->lock_);
  return proc;
}


// ---------------------------------------------------------------------------
void destroy_process (kProcess_t* process)
{
  // assert(proc->flags_ & TK_EXITED);
  // assert(no watcher);

  klock (&process->lock_);
  kThread_t* task;
  for_each (task, &process->threads_, kThread_t, taskNd_) {
    kThread_t* pick = task;
    destroy_thread (pick);
  }

  klist_remove (&kSYS.processes_, &process->allNd_);
  --process->session_->user_->processCount_;
  if (--process->session_->processCount_ <= 0)
    destroy_session (process->session_);
  kunlock (&process->lock_);
  kfree(process);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
