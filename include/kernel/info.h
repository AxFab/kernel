#ifndef KINFO_H__
#define KINFO_H__

#ifndef KCORE_H__
#  error You must include kernel/core.h before of kernel/info.h
#endif

#include <kernel/cpu.h>
#include <smkos/spinlock.h>
#include <smkos/llist.h>
#include <smkos/bbtree.h>
#ifdef __KERNEL
#  include <ax/alloc.h>
#endif


// ---------------------------------------------------------------------------
typedef struct kCpuCore kCpuCore_t;
struct kCpuCore {
  // CORE
  int   cpuNo_;
  int   errNo;
  int   lockCounter;
  int   tmpPageStack_;
  int   ready_;
  // INODES
  // RootInode

  // MEMORY
  // AddressSpace

  // SCHEDULER
  nanotime_t     lastStatus_;
  int         ticksCount_;
  int         state_;
  nanotime_t     stateTime_ [ CPU_STATE_COUNT ];
  kThread_t    *current_;

  // STATS
  int         statistics_ [ CPU_STATE_COUNT ];

};

// ---------------------------------------------------------------------------
typedef struct kSysCore kSysCore_t;
struct kSysCore {
  // CORE
  nanotime_t     now_;       // IMPORTANT! Must be the first field for ASM timers
  int         state_;
  int         cpuCount_;

#ifdef __KERNEL
  xHeapArea_t   kheap;
#endif

  // MMU
  long pageAvailable_;
  long pageMax_;
  uint64_t memMax_;

  long          pidAutoInc_;
  long          taskAutoInc_;

  struct llhead      userList_;

  struct llhead      devices_;

  kProcess_t *execStart_;
  struct llhead    processes_;
  struct llhead    waitList_;

  // INODES
  int         autoIno_;
  int         autoPipe_;
  kInode_t   *rootNd_;
  kInode_t   *devNd_;
  kInode_t   *mntNd_;
  kInode_t   *pipeNd_;
  struct llhead    inodeLru_;

  // MEMORY

  // SCHEDULER
  int         autoPid_;
  int         ticksCountMax_;
  int         schedLatency_;
  int         minTimeSlice_;
  struct spinlock  schedLock_;
  int         tasksCount_ [ SCHED_COUNT ];
  struct spinlock  procLock_;
  kProcess_t *allProcFrst_;
  kProcess_t *allProcLast_;
  kThread_t    *allTaskFrst_;
  kThread_t    *allTaskLast_;
  int         prioWeight_;
  struct spinlock  timerLock_;
  nanotime_t     timerMin_;

  // STATS
  float       loadAvg_ [ KRN_LOADAVG_COUNT ];
  float       loadCoef_ [ KRN_LOADAVG_COUNT ];

  // kCpuCore_t cpus_ [0];
};

// ---------------------------------------------------------------------------
// typedef struct kHdwCore kHdwCore_t;
// struct kHdwCore
// {
//   size_t      userSpaceBase_;
//   size_t      userSpaceLimit_;

//   size_t      pageBitmapAdd_;
//   ssize_t     pageBitmapLg_;

//   uint32_t*   kernelDir_;
//   uint32_t*   kernelTbl0_;
//   uint32_t*   screenTbl_; // FIXME - only for early implement iteration
// };

// ---------------------------------------------------------------------------
int kCpu_SetStatus (int state);
void kCpu_Statistics ();
int kSys_NewPid();
int kSys_NewIno();

// ---------------------------------------------------------------------------
// extern kCpuCore_t kCPU;
// extern kSysCore_t kSYS;
// extern kHdwCore_t kHDW;

#endif /* KINFO_H__ */
