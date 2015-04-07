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
#include <kernel/stream.h>


// ===========================================================================

extern kStream_t *keyboard_tty;

int process_login(void *user, kInode_t *prg, kInode_t *dir, kInode_t *tty, const char *cmd)
{
  // assert (user != NULL);
  assert (prg != NULL);
  assert (dir != NULL);
  assert (tty != NULL);
  assert (cmd != NULL);

  kAssembly_t *image = load_assembly (prg);

  if (image == NULL)
    return __geterrno();


  // FIXME load the image
  kProcess_t *proc = KALLOC (kProcess_t);
  addspace_init (&proc->memSpace_, 0);
  // kAddSpace_t* mmsp = kvma_new (4 * _Kb_);
  map_assembly (&proc->memSpace_, image);
  // kasm_load (&proc->memSpace_, prg);

  proc->pid_ = ++kSYS.pidAutoInc_;
  proc->execStart_ = kSYS.now_;
  proc->workingDir_ = dir;
  proc->parent_ = NULL;
  proc->image_ = prg;
  proc->command_ = kstrdup(cmd);
  // proc->memSpace_ = mmsp;

  proc->streamCap_ = 0;
  proc->openStreams_ = NULL; // (kStream_t**)kalloc (sizeof(kStream_t*) * 8);
  stream_tty (proc, tty);

  if (keyboard_tty == NULL)
    keyboard_tty = proc->openStreams_[0];

  // proc->openStreams_[0] = stream_set(tty, O_RDONLY);
  // proc->openStreams_[1] = stream_set(tty, O_WRONLY);
  // proc->openStreams_[2] = stream_set(tty, O_WRONLY);

  klock (&proc->lock_, LOCK_PROCESS_CREATION);
  klist_push_back(&kSYS.processes_, &proc->procNd_);
  kThread_t *task = ksch_new_thread (proc, 0x1000000, 0xc0ffee);
  klist_push_back(&proc->threads_, &task->taskNd_);

  kunlock (&proc->lock_);
  kprintf ("Start login program [%d - %s - root<0> ]\n", proc->pid_, prg->name_);
  return proc->pid_;
}



// ===========================================================================
// ---------------------------------------------------------------------------
/** Create a new process, ready to start */
int ksch_create_process (kProcess_t *parent, kInode_t *image, kInode_t *dir, const char *cmd)
{
  assert (image != NULL);
  assert (parent != NULL);

  kAssembly_t *asmimage = load_assembly (image);

  if (asmimage == NULL)
    return __geterrno();


  // FIXME load the image
  kProcess_t *proc = KALLOC (kProcess_t);
  addspace_init (&proc->memSpace_, 0);
  // kAddSpace_t* mmsp = kvma_new (4 * _Kb_);
  map_assembly (&proc->memSpace_, asmimage);
  // kasm_load (&proc->memSpace_, image);

  proc->pid_ = ++kSYS.pidAutoInc_;
  proc->execStart_ = kSYS.now_;
  proc->workingDir_ = dir;
  proc->parent_ = parent;
  proc->image_ = image;
  proc->command_ = kstrdup(cmd);
  // proc->memSpace_ = mmsp;
  atomic_inc_i32 (&parent->childrenCount_);

  proc->streamCap_ = 8;
  proc->openStreams_ = (kStream_t **)kalloc (sizeof(kStream_t *) * 8, 0);
  // @todo we must clone the stream and check if still here !
  proc->openStreams_[0] = parent->openStreams_[0]; // (kStream_t*)1;
  proc->openStreams_[1] = parent->openStreams_[1]; // kstm_create_pipe (O_WRONLY, 4 * _Kb_);
  proc->openStreams_[2] = parent->openStreams_[2]; // proc->openStreams_[1];

  klock (&proc->lock_, LOCK_PROCESS_CREATION);
  klist_push_back(&kSYS.processes_, &proc->procNd_);

  kThread_t *task = ksch_new_thread (proc, 0x1000000, 0xc0ffee);
  klist_push_back(&proc->threads_, &task->taskNd_);

  kunlock (&proc->lock_);
  kprintf ("Start program [%d - %s - root<0> - /mnt/cd0/USR/BIN]\n", proc->pid_, image->name_);
  return proc->pid_;
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
void ksch_destroy_process (kProcess_t *proc)
{
  proc->flags_ |= TK_EXITED;
  // FIXME if (proc->parent_ != NULL && proc->flags_ & PROC_TRACED) { // signal() }

  klock (&proc->lock_, LOCK_PROCESS_DESTROY);
  kThread_t *task; // = proc->threadFrst_;
  for_each (task, &proc->threads_, kThread_t, taskNd_) {
    // while (task != NULL) {
    kThread_t *pick = task;
    // task = task->nextPr_;
    ksch_destroy_thread (pick);
  }

  // proc->threadFrst_ = NULL;
  // proc->threadLast_ = NULL;

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
void ksch_exit (kProcess_t *proc, int status)
{
  proc->flags_ |= TK_EXITED;
  proc->exitStatus_ = status;
  kThread_t *task; // = proc->threadFrst_;
  for_each (task, &proc->threads_, kThread_t, taskNd_) {
    ksch_abort (task);
  }
}

