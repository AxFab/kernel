#ifndef TASKS_H__
#define TASKS_H__

#include <kcore.h>

// ============================================================================

#define TASK_CLASS_IDLE       0
#define TASK_CLASS_NORMAL     1
#define TASK_CLASS_HIGH       2
#define TASK_CLASS_CRITICAL   3
#define TASK_CLASS_COUNT      4

// ============================================================================
struct kWorkspace {
  // int             pathsCap_;
  // kInode_t**      paths_;

  kProcess_t*     processHead_t;
  // kInode_t*       root_;
  // kUser_t*        user_;
};

struct kProcess {
  pid_t           pid_;
  kWorkspace_t*   workspace_;
  kThread_t*      threadHead_;
  kAddSpace_t*    maddress_;

  kProcess_t*     parent_;
  kProcess_t*     nextSibl_;
  kProcess_t*     prevSibl_;
  kProcess_t*     firstChild_;


  kInode_t*       workingDir_;
  kAssembly_t*    assembly_;
  kInode_t*       files_[32];

  kThread_t       thread_;


};


struct kThread {

  kProcess_t*     process_;
  kThread_t*      nextSibling_;
  kThread_t*      next_;
  kThread_t*      prev_;
  int             class_;
  long           elapseSleep_;
  long           elapseUser_;
  long           elapseKernel_;

};

struct kAssembly {
  void* entry_;
  size_t stackSize_;
  kSection_t* section_;
};

// ============================================================================

// CONTEXT
pid_t kTsk_GetPid();


// SCHEDULER
void kSch_Initialize ();
int kSch_AppendThread ();
int kSch_SetPriority ();
int kSch_SetClass ();
int kSch_SetCpuAffinity ();
int kSch_SaveThread ();
void kSch_Ticks ();

// WORKSPACES
kWorkspace_t* kTsk_NewWorkspace();

// THREAD
kThread_t* kTsk_NewThread (kProcess_t* proc, void* stackPtr);

// PROCESS
kProcess_t* kTsk_NewProcess (kAssembly_t* assembly, kInode_t* dir);

// EVENT
int kTsk_WaitFor (kThread_t* qw, int what, void* param);
int kTsk_Event (int what, void* param);

// ASSEMBLY
kAssembly_t* kAsm_Open (kInode_t* ino);



#endif /* TASKS_H__ */
