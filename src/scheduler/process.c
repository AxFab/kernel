#include <scheduler.h>
#include <memory.h>
#include <inodes.h>
#include <kinfo.h>



// ---------------------------------------------------------------------------
/** Attach a new process to the system list.
 * \note {proc} must be locked
 */
static void kSch_AttachProcess (kProcess_t *proc)
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

// ===========================================================================
/** Create a new process, ready to start */
int kSch_NewProcess (kProcess_t* parent, kInode_t* image, kInode_t* dir)
{
  assert (image != NULL);

  if (!kAsm_Open (image)) {
    __seterrno (ENOEXEC);
    return 0;
  }

  // FIXME load the image
  kAddSpace_t* mmsp = kVma_New (4 * _Mb_);
  kAsm_Load (mmsp, image);

  kProcess_t* proc = KALLOC (kProcess_t);
  proc->pid_ = kSys_NewPid();
  proc->execStart_ = ltime(NULL);
  proc->workingDir_ = dir;
  proc->parent_ = parent;
  proc->image_ = image;
  proc->memSpace_ = mmsp;
  if (parent)  atomic_inc_i32 (&parent->childrenCount_);

  proc->streamCap_ = 8;
  proc->openStreams_ = (kStream_t**)kalloc (sizeof(kStream_t*) * 8);
  proc->openStreams_[0] = (kStream_t*)1;
  proc->openStreams_[1] = (kStream_t*)1;
  proc->openStreams_[2] = (kStream_t*)1;


  klock (&proc->lock_, LOCK_PROCESS_CREATION);
  kSch_AttachProcess (proc);
  proc->threadCount_++;
  proc->threadFrst_ = kSch_NewThread (proc, 0x1000000, 0xc0ffee);
  proc->threadLast_ = proc->threadFrst_;



  kunlock (&proc->lock_);

  kprintf ("Start program [%d - %s - root<0> - /mnt/cd0/USR/BIN]\n", proc->pid_, image->name_);

  return proc->pid_;
}

// ---------------------------------------------------------------------------
/** Add a new thread to the process */
int kSch_AddThread (kProcess_t* proc, uintptr_t entry, intmax_t arg)
{
  assert (proc != NULL);
  assert (kCPU.current_ != NULL && proc == kCPU.current_->process_);

  if (proc->flags_ & PROC_EXITED)
    return __seterrno(ENOSYS);

  klock (&proc->lock_, LOCK_PROCESS_ADD_THREAD);
  kTask_t* pick = proc->threadFrst_;
  while (pick != NULL) {
    if (pick->state_ == TASK_STATE_ZOMBIE) {
      kSch_ResurectTask (pick, entry, arg);
      kunlock (&proc->lock_);
      return __noerror();
    }
    pick = pick->nextPr_;
  }

  proc->threadCount_++;
  proc->threadLast_->nextPr_ = kSch_NewThread (proc, entry, arg);
  proc->threadLast_ = proc->threadLast_->nextPr_;
  kunlock (&proc->lock_);
  return __noerror();
}

// ---------------------------------------------------------------------------
/** Push process to the garbage collector */
void kSch_DestroyProcess (kProcess_t* proc)
{
  proc->flags_ |= PROC_EXITED;
  // FIXME if (proc->parent_ != NULL && proc->flags_ & PROC_TRACED) { // signal() }

  klock (&proc->lock_, LOCK_PROCESS_DESTROY);
  kTask_t* task = proc->threadFrst_;
  while (task != NULL) {
    kTask_t* pick = task;
    task = task->nextPr_;
    kSch_DestroyThread (pick);
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
 *  kSch_DestroyProcess is called.
 */
void kSch_ExitProcess (kProcess_t* proc, int status)
{
  proc->flags_ |= PROC_EXITED;
  proc->exitStatus_ = status;
  kTask_t* task = proc->threadFrst_;
  while (task != NULL) {
    kSch_Abort (task);
    task = task->nextPr_;
  }
}

