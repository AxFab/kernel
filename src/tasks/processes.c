#include <tasks.h>
#include <memory.h>

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


kAddSpace_t* kVma_New (size_t stack_size)
{
  return NULL;
}

