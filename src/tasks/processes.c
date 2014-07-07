#include <tasks.h>
#include <memory.h>

// ============================================================================
kProcess_t* kTsk_NewProcess (char* filename, kInode_t* output, kInode_t* input) 
{
  kProcess_t* proc;

  proc = KALLOC(kProcess_t);
  // Command line arguments
  proc->argc_ = 0;
  proc->argv_ = NULL;

  // Get Workspace
  proc->workspace_ = kGetCurrent()->process_->workspace_;
  proc->workdir_ = proc->workspace_->directory;
  proc->parent_ = kGetCurrent()->process_;
  proc->flags_ = 0; /* PROC_NEW_WINDOW | PROC_LOGIN | PROC_DIALG */

  proc->filename_ = "/bin/bash";
  proc->files_[0] = output;
  proc->files_[1] = input;
  proc->files_[2] = error;
}



// ============================================================================

void kTsk_SetThread (void* entry, uintmax_t param)
{
}

kProcess_t* kTsk_NewProcess (kAssembly_t* assembly, kInode_t* dir)
{
  kProcess_t* proc;
  assert (assembly && dir);
  proc = KALLOC(kProcess_t);
  proc->pid_ = kTsk_GetPid();
  proc->parent_ = NULL;
  proc->workspace_ = kTsk_NewWorkspace();
  proc->workingDir_ = dir;
  proc->assembly_ = assembly;
  proc->threadHead_ = kTsk_NewThread (proc, (void*)0xd0000000 - 0x10);
  proc->maddress_ = kVma_New(assembly->stackSize_);
  kTsk_SetThread (assembly->entry_, 0xcafebabe);
  return proc;
}

kThread_t* kTsk_NewThread (kProcess_t* proc, void* stackPtr)
{
  kThread_t* thread;
  assert (proc && stackPtr);
  thread = KALLOC(kThread_t);
  thread->process_ = proc;

  thread->nextSibling_ = proc->threadHead_;
  proc->threadHead_ = thread;
  return thread;
}


kWorkspace_t* kTsk_NewWorkspace ()
{
  return NULL;
}

pid_t kTsk_GetPid()
{
  return 0;
}

