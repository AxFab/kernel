#ifndef SCHEDULER_H__
#define SCHEDULER_H__

#include <kernel/core.h>
#include <kernel/cpu.h>

// Process flags
#define PROC_EXITED     (1 << 0)

// Thread flags
#define TASK_REMOVED    (1 << 0)


struct kProcess
{
  int           pid_;             /// Unique number
  int           flags_;           /// Process flags [ EXITED ]
  int           childrenCount_;   /// Number of child processes
  int           threadCount_;     /// Number of allocated threads
  int           runningTask_;     /// Number of thread running
  kProcess_t*   parent_;          /// Parent process - the one that starts it (should be a task !?)
  kProcess_t*   nextAll_;         /// Next process on total lists
  kTask_t*      threadFrst_;      /// First thread on the list
  kTask_t*      threadLast_;      /// Last thread on the list
  ltime_t       execStart_;       /// DateTime of the start of this process
  int           exitStatus_;      /// status of this process at exit
  spinlock_t    lock_;            /// Lock
  kInode_t*     image_;
  kInode_t*     workingDir_;
  kAddSpace_t*  memSpace_;
  uint32_t      dir_;
  kStream_t**   openStreams_;
  int           streamCap_;
};

struct kTask
{
  int           tid_;             /// Unique number
  int           flags_;           /// Thread flags [ ]
  int           execOnCpu_;       /// Which CPU is currently running this task
  int           timeSlice_;
  int           niceValue_;
  int           state_;
  int           eventType_;
  uint64_t      eventParam_;

  ltime_t       lastWakeUp_;
  ltime_t       elapsedUser_;
  ltime_t       elapsedSystem_;
  ltime_t       execStart_;


  kProcess_t*   process_;
  kTask_t*      nextSc_;          /// Next item on scheduler list
  kTask_t*      nextPr_;          /// Next item on process list
  kTask_t*      prevEv_;          /// Previous item on event list
  kTask_t*      nextEv_;          /// Next item on event list

  spinlock_t    lock_;            /// Lock

  kCpuRegs_t    regs_;
  uint32_t      kstack_;
};



// ---------------------------------------------------------------------------
void kSch_WaitEvent(kTask_t* task, int event, long param, kCpuRegs_t* regs);
void kSch_CancelEvent (kTask_t* task);
// ---------------------------------------------------------------------------
// int kSch_TimeSlice (kTask_t* task);
void kSch_Ticks (kCpuRegs_t* regs) ;
void kSch_PickNext ();
// ---------------------------------------------------------------------------
int kSch_NewProcess (kProcess_t* proc, kInode_t* image, kInode_t* dir);
int kSch_AddThread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void kSch_DestroyProcess (kProcess_t* proc);
void kSch_ExitProcess (kProcess_t* proc, int status);
// ---------------------------------------------------------------------------
void kSch_Initialize ();
int kSch_OnTask ();
void kSch_WakeUp (kTask_t* task);
void kSch_StopTask (int state, kCpuRegs_t* regs);
void kSch_Abort (kTask_t* task);
// ---------------------------------------------------------------------------
kTask_t* kSch_NewThread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void kSch_ResurectTask (kTask_t* task, uintptr_t entry, intmax_t arg) ;
void kSch_DestroyThread (kTask_t* task) ;
// ---------------------------------------------------------------------------




#endif /* SCHEDULER_H__ */
