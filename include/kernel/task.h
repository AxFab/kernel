#ifndef KERNEL_TASK_H__
#define KERNEL_TASK_H__

#include <kernel/core.h>
#include <kernel/memory.h>
#include <kernel/cpu.h> // TEMPORARY

#define TK_EXITED  (1 << 0) // For a process that have stop running.
#define TK_REMOVED (1 << 1) // For a thread/process of waiting to be destroy


#define SCHED_ZOMBIE     0
#define SCHED_SLEEPING   1
#define SCHED_WAITING    2
#define SCHED_READY      3
#define SCHED_RUNNING    4


struct kUser 
{
  const char*   name_;
  int           privileges_;
  llnode_t        allNd_;
  int           processCount_;
};

struct kThread
{
  int           taskId_;          /// Unique number
  spinlock_t    lock_;            /// Lock
  int           flags_;           /// Thread flags [ ]

  int           execOnCpu_;       /// Which CPU is currently running this task
  int           timeSlice_;
  int           niceValue_;
  int           state_;

  nanotime_t    lastWakeUp_;
  nanotime_t    elapsedUser_;
  nanotime_t    elapsedSystem_;
  nanotime_t    execStart_;

  kProcess_t*   process_;
  kThread_t*    nextSc_;          /// Next item on scheduler list

  llnode_t        taskNd_;
  void*         execPointer_;
  long          param_;
  bool          restart_;
  uint32_t      ustack_;
  uint32_t      kstack_;

  // TO DELETE
  llnode_t        procNd_;         /// Node to connect on  process list
  kWaiting_t*   event_;
  kCpuRegs_t    regs_;
};


struct kProcess 
{
  int           pid_;         /// Unique number
  spinlock_t    lock_;
  int           flags_;       /// Process flags

  kProcess_t*   parent_;      /// Parent process - the one that starts it
  llhead_t      children_;    /// List of children process.
  llnode_t        siblingNd_;
  llnode_t        allNd_;       /// List of every process on a system

  kInode_t*     workingDir_;  /// Working directory.
  kAssembly_t*  asmImg_;      /// 
  llhead_t      threads_;
  nanotime_t    execStart_;
  kAddSpace_t   memSpace_;
  int           runningTask_;
  kSession_t*   session_;

  // Statistics about memory =========================
  long          privatePage_;  /// Number of private page for the process (Heap, table, stack, file_copy).


  // TO DELETE
  int           childrenCount_;   /// Number of child processes
  int           threadCount_;     /// Number of allocated threads
  llnode_t        procNd_;          /// Node to connect on procList
  int           exitStatus_;      /// status of this process at exit
 
  kInode_t*     image_;
  const char*   command_;
  kStream_t**   openStreams_;
  int           streamCap_;

  uint32_t      dir_;
  page_t        pageDir_;
};


struct kAssembly 
{
  int         flags_;
  kInode_t*   ino_;
  void*       entryPoint_;
  llhead_t    sections_;

  size_t      stackSize_;
};

struct kSection
{
  uintptr_t   address_;
  uintptr_t   length_;
  uintptr_t   align_;
  uintptr_t   offset_;
  kSection_t* next_ ;
  int         flags_;
  llnode_t      node_;
};


int addspace_init(kAddSpace_t* space, int flags);
kVma_t* vmarea_append(kAddSpace_t* space, size_t length, int flags);
kVma_t* vmarea_map_exec(kAddSpace_t* space, kSection_t* section);

void sched_insert(kThread_t* thread);
void sched_wakeup(kThread_t* thread);
void sched_remove(kThread_t* thread);

// ===========================================================================

// TASK/USER =================================================================
kUser_t* create_user (const char* name, int privileges);
kUser_t* search_user (const char* name, const char* domain);
void destroy_user (kUser_t* user);

// TASK/ASSEMBLY =============================================================
/** Read an image file and create the corresponding assembly. */
kAssembly_t* load_assembly (kInode_t* ino);
void destroy_assembly (kAssembly_t* image);
/** Load an assembly image on an address space. */
int map_assembly (kAddSpace_t* mspace, kAssembly_t* image);

// TASK/PROCESS ==============================================================
/** Create a parentless process with a new user */
kProcess_t* login_process (kAssembly_t* asmImg, kUser_t* user, kInode_t* dir, kInode_t* term, const char* cmd);
/** Create a new process on the same session of the parent */
kProcess_t* create_process (kAssembly_t* asmImg, const char* cmd, const char* env);
/** */
void destroy_process (kProcess_t* proc);

// TASK/THREAD ===============================================================
/**  */
kThread_t* create_thread (kProcess_t* proc, void* pointer, long param);
/** Add a new thread to the process */
kThread_t* append_thread (kProcess_t* proc, void* pointer, long param);
/** */
void destroy_thread (kThread_t* thread);


// TASK/SECURITY =============================================================
int capability_for_inode (kInode_t* ino, int action);
int capability_for_process (kProcess_t* process, int action);



// void thread_reset (void* entry, long param, long errno);




#endif /* KERNEL_TASK_H__ */
