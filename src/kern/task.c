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
 *      Process, thread and session managment.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>

/* ----------------------------------------------------------------------- */
static kThread_t *alloc_thread(kProcess_t *process)
{
  int stackFlags = VMA_STACK | VMA_READ | VMA_WRITE;
  kThread_t *thread;

  assert (process != NULL);
  assert (kislocked(&process->lock_));

  thread = KALLOC(kThread_t);
  thread->process_ = process;
  klock(&process->mspace_.lock_);
  thread->kstack_ = area_map(&process->mspace_, PAGE_SIZE * 2, stackFlags);
  thread->ustack_ = area_map(&process->mspace_, 1 * _Mb_, stackFlags);
  kunlock(&process->mspace_.lock_);

  ll_push_back(&process->threads_, &thread->taskNd_);

  thread->state_ = SCHED_ZOMBIE;
  return thread;
}


/* ----------------------------------------------------------------------- */
/**  */
static void reset_thread (kThread_t *thread, size_t entry, size_t param)
{
  assert (thread->state_ == SCHED_ZOMBIE);

  thread->start_ = time(NULL);
  thread->paramEntry_ = entry;
  thread->paramValue_ = param;
  atomic_inc(&thread->process_->runningTask_);
  thread->state_ = SCHED_READY;
  // sched_wakeup (thread);
  sched_insert (kSYS.scheduler_, thread);

  /* thread->execOnCpu_ = -1;
  thread->niceValue_ = 5; */
  // sched_insert (thread);
}


/* ----------------------------------------------------------------------- */
static kSession_t *alloc_session(kUser_t *user, kInode_t *dir)
{
  kSession_t *session;
  if (inode_open(dir))
    return NULL;

  session = KALLOC(kSession_t);
  session->user_ = user;
  session->workingDir_ = dir;
  return session;
}


/* ----------------------------------------------------------------------- */
static kProcess_t *alloc_process(kAssembly_t* assembly, int pid)
{
  kThread_t *thread;
  kProcess_t *process = KALLOC (kProcess_t);
  process->pid_ = pid;
  // process->commandLine_ = strdup(cmd);
  process->start_ = time(NULL);
  inode_open(assembly->ino_);
  process->assembly_ = assembly;
  atomic_inc(&assembly->usage_);
  mmu_map_userspace(&process->mspace_);

  klock (&process->lock_);
  ll_push_back(&kSYS.processes_, &process->allNd_);

  thread = alloc_thread (process);
  reset_thread (thread, assembly->entryPoint_, 0xc0ffee);
  return process;
}


/* ----------------------------------------------------------------------- */
static void destroy_thread(kThread_t *thread)
{
  assert(thread->state_ == SCHED_ZOMBIE);
  klock(&thread->process_->mspace_.lock_);
  area_unmap (&thread->process_->mspace_, thread->kstack_);
  area_unmap (&thread->process_->mspace_, thread->ustack_);
  kunlock(&thread->process_->mspace_.lock_);
  kfree(thread);
}


/* ----------------------------------------------------------------------- */
static void destroy_session(kSession_t *session)
{
  inode_close(session->workingDir_);
  kfree(session);
}


/* ----------------------------------------------------------------------- */
kThread_t *create_thread(kProcess_t *process, size_t entry, size_t param)
{
  kThread_t *thread;

  assert (process != NULL);
  // assert (kCPU.current_ != NULL && process == kCPU.current_->process_);

  // __seterrno(0);
  // if (process->flags_ & TK_REMOVED)
  //   return __seterrnoN(ENOSYS, kThread_t);

  klock (&process->lock_);
  ll_for_each (&process->threads_, thread, kThread_t, taskNd_) {
    if (thread->state_ == SCHED_ZOMBIE)
      break;
  }

  if (thread == NULL)
    thread = alloc_thread (process);
  reset_thread (thread, entry, param);
  kunlock (&process->lock_);
  return thread;
}


/* ----------------------------------------------------------------------- */
void destroy_process (kProcess_t *process)
{
  kThread_t *task;
  kThread_t *pick;

  assert(kislocked(&process->lock_));
  // @todo free process, threads, resx, session, events, ...
  
  task = ll_first(&process->threads_, kThread_t, taskNd_);
  while (task) {
    pick = task;
    task = ll_next(task, kThread_t, taskNd_);
    destroy_thread (pick);
  }

  ll_remove(&kSYS.processes_, &process->allNd_);
  if (atomic_add(&process->session_->usage_, -1) <= 0)
    destroy_session(process->session_);
  
  atomic_dec(&process->assembly_->usage_);
  inode_close(process->assembly_->ino_);
  kunlock(&process->lock_);
  kfree(process);
}

/* ----------------------------------------------------------------------- */
kProcess_t *create_logon_process(kInode_t* ino, kUser_t* user, kInode_t* dir, const char*cmd)
{
  kProcess_t *process;
  kInode_t* stdin;
  kInode_t* stdout;
  kInode_t* stderr;
  kInode_t* procdir;
  char bufPid[12];
  int pid = ++kSYS.pidAutoInc_;

  assert(ino != NULL);
  assert(user != NULL);
  assert(dir != NULL);
  assert(cmd != NULL);

  snprintf(bufPid, 12, "%d", pid);
  procdir = create_inode (bufPid, kSYS.procIno_, S_IFDIR | 0400, 0);
  stdin = create_inode ("stdin", procdir, S_IFIFO | 0400, PAGE_SIZE);
  stdout = create_inode ("stdout", procdir, S_IFIFO | 0400, PAGE_SIZE);
  stderr = stdout;

  process = alloc_process(ino->assembly_, pid);
  if (process == NULL)
    return NULL;

  process->session_ = alloc_session (user, dir);
  atomic_inc(&process->session_->usage_);

  if (area_assembly(&process->mspace_, ino->assembly_)) {
    destroy_process (process);
    return NULL;
  }

  process_set_resx(process, stdin, 0); // O_RDLY | 
  process_set_resx(process, stdout, 0); // O_WRLY | 
  process_set_resx(process, stderr, 0); // O_WRLY | O_DIRECT

  open_subsys(/*subsys, */stdin, stdout);

  kunlock (&process->lock_);
  
  return process;
}


/* ----------------------------------------------------------------------- */
kProcess_t *create_child_process(kInode_t* ino, kProcess_t* parent, const char*cmd)
{
  kProcess_t *process;
  // kInode_t* stdin;
  // kInode_t* stdout;
  // kInode_t* stderr;
  kInode_t* procdir;
  char bufPid[12];
  int pid = ++kSYS.pidAutoInc_;

  assert(ino != NULL);
  assert(parent != NULL);
  assert(cmd != NULL);

  snprintf(bufPid, 12, "%d", pid);
  procdir = create_inode (bufPid, kSYS.procIno_, S_IFDIR | 0400, 0);
  ((void)procdir);
  // stdin = create_inode ("stdin", procdir, S_IFIFO | 0400, PAGE_SIZE);
  // stdout = create_inode ("stdout", procdir, S_IFIFO | 0400, PAGE_SIZE);
  // stderr = stdout;

  process = alloc_process(ino->assembly_, pid);
  if (process == NULL)
    return NULL;

  ll_push_back(&parent->children_, &process->siblingNd_);
  process->session_ = parent->session_;
  atomic_inc(&process->session_->usage_);

  if (area_assembly(&process->mspace_, ino->assembly_)) {
    kunlock (&process->lock_);
    destroy_process (process);
    return NULL;
  }

  kunlock (&process->lock_);
  return process;
}


/* ----------------------------------------------------------------------- */
void thread_abort (kThread_t* thread)
{
  assert(kislocked(&thread->process_->lock_));
  assert(thread != kCPU.current_);

  if (thread->state_ == SCHED_EXEC) {
    thread->state_ = SCHED_ABORT;

  } else if (thread->state_ != SCHED_ZOMBIE) {
    if (thread->state_ == SCHED_BLOCKED) {
      // async_cancel_event(task);
    }

    sched_remove (kSYS.scheduler_, thread);
    thread->state_ = SCHED_ZOMBIE;
    atomic_dec (&thread->process_->runningTask_);

    // @todo And all signal have been sended...
    if (thread->process_->runningTask_ == 0) {
      destroy_process (thread->process_);
    }
  }
}


/* ----------------------------------------------------------------------- */
void process_exit(kProcess_t *process, int status)
{
  kThread_t *task;

  klock(&process->lock_);
  process->exitStatus_ = status;
  ll_for_each (&process->threads_, task, kThread_t, taskNd_) {
    assert(kCPU.current_ != task);
    thread_abort (task);
  }

  kunlock(&process->lock_);
}


/* ----------------------------------------------------------------------- */
kResx_t *process_get_resx(kProcess_t *process, int fd, int access)
{
  kResx_t* resx;
  klock(&process->lock_);
  resx = bb_search (&process->resxTree_, fd, kResx_t, fdNd_);
  if (resx == NULL) {
    kunlock(&process->lock_);
    return __seterrnoN(EBADF, kResx_t);
  }

  assert ((int)resx->fdNd_.value_ == fd);
  // @todo check capacity (access rights)

  kunlock(&process->lock_);
  return resx;
}


/* ----------------------------------------------------------------------- */
kResx_t *process_set_resx(kProcess_t *process, kInode_t* ino, int oflags)
{
  kResx_t* resx;

  assert (kislocked(&process->lock_));
  if (process->fdCount_ > MAX_FD_PER_PROCESS)
    return NULL;
  

  resx = KALLOC(kResx_t);
  resx->ino_ = ino;
  resx->type_ = ino->stat_.mode_ & S_IFMT;
  resx->oflags_ = oflags;
  // @todo check capacity (access rights)
  resx->fdNd_.value_ = process->fdCount_++;
  bb_insert (&process->resxTree_, &resx->fdNd_);
  return resx;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
