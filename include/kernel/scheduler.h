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
  list_t        procNd_;          /// Node to connect on procList
  anchor_t      threadList_;      /// Collection of thread from this process
  // kTask_t*      threadFrst_;      /// First thread on the list
  // kTask_t*      threadLast_;      /// Last thread on the list
  ltime_t       execStart_;       /// DateTime of the start of this process
  int           exitStatus_;      /// status of this process at exit
  spinlock_t    lock_;            /// Lock
  kInode_t*     image_;
  kInode_t*     workingDir_;
  kAddSpace_t*  memSpace_;
  const char*   command_;
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
  // int           eventType_;
  // uint64_t      eventParam_;

  ltime_t       lastWakeUp_;
  ltime_t       elapsedUser_;
  ltime_t       elapsedSystem_;
  ltime_t       execStart_;

  kProcess_t*   process_;
  kTask_t*      nextSc_;          /// Next item on scheduler list
  list_t        procNd_;         /// Node to connect on  process list

  kWaiting_t*   event_;
  // list_t        eventNd_;      /// Node to connect on event_list


  spinlock_t    lock_;            /// Lock

  kCpuRegs_t    regs_;
  uint32_t      kstack_;
  uint32_t      ustack_;
};



// ---------------------------------------------------------------------------
void kevt_wait(kTask_t* task, int event, long param, kCpuRegs_t* regs);
void kevt_cancel (kTask_t* task);
// ---------------------------------------------------------------------------
// int ksch_timeslice (kTask_t* task);
void ksch_ticks (kCpuRegs_t* regs) ;
void ksch_pick ();
// ---------------------------------------------------------------------------
int ksch_create_process (kProcess_t* proc, kInode_t* image, kInode_t* dir, const char* cmd);
int process_login(void* user, kInode_t* prg, kInode_t* dir, kInode_t* tty, const char* cmd);
int ksch_add_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void ksch_destroy_process (kProcess_t* proc);
void ksch_exit (kProcess_t* proc, int status);
// ---------------------------------------------------------------------------
void ksch_init ();
int ksch_ontask ();
void ksch_wakeup (kTask_t* task);
void ksch_stop (int state, kCpuRegs_t* regs);
void ksch_abort (kTask_t* task);
// ---------------------------------------------------------------------------
kTask_t* ksch_new_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void ksch_resurect_thread (kTask_t* task, uintptr_t entry, intmax_t arg) ;
void ksch_destroy_thread (kTask_t* task) ;
// ---------------------------------------------------------------------------

/** The event ticks check if some timers are expired. */
void kevt_ticks();
void task_pause();

#endif /* SCHEDULER_H__ */
