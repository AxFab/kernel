#ifndef KINFO_H__
#define KINFO_H__

#include <kernel/core.h>
#include <kernel/cpu.h>
#ifdef __KERNEL
#  include <alloc.h>
#endif


// ---------------------------------------------------------------------------
typedef struct kCpuCore kCpuCore_t;
struct kCpuCore
{
  // CORE
  int   cpuNo_;
  int   errNo;
  int   lockCounter;

  // INODES
  // RootInode

  // MEMORY
  // AddressSpace

  // SCHEDULER
  ltime_t     lastStatus_;
  int         ticksCount_;
  int         state_;
  ltime_t     stateTime_ [ CPU_STATE_COUNT ];
  kTask_t*    current_;

  // STATS
  int         statistics_ [ CPU_STATE_COUNT ];

};

// ---------------------------------------------------------------------------
typedef struct kSysCore kSysCore_t;
struct kSysCore
{
  // CORE
  ltime_t     now_;       // IMPORTANT! Must be the first field for ASM timers
  int         state_;
  int         cpuCount_;

#ifdef __KERNEL
  xHeapArea_t   kheap;
#endif

  // INODES
  int         autoIno_;
  int         autoPipe_;
  kInode_t*   rootNd_;
  kInode_t*   devNd_;
  kInode_t*   mntNd_;
  kInode_t*   pipeNd_;

  // MEMORY

  // SCHEDULER
  int         autoPid_;
  int         ticksCountMax_;
  int         schedLatency_;
  int         minTimeSlice_;
  spinlock_t  schedLock_;
  int         tasksCount_ [ TASK_STATE_COUNT ];
  spinlock_t  procLock_;
  kProcess_t* allProcFrst_;
  kProcess_t* allProcLast_;
  kTask_t*    allTaskFrst_;
  kTask_t*    allTaskLast_;
  int         prioWeight_;
  spinlock_t  timerLock_;
  ltime_t     timerMin_;
  kTask_t*    timerFrst_;
  kTask_t*    timerLast_;

  // STATS
  float       loadAvg_ [ KRN_LOADAVG_COUNT ];
  float       loadCoef_ [ KRN_LOADAVG_COUNT ];

  // kCpuCore_t cpus_ [0];
};


// ---------------------------------------------------------------------------
int kCpu_SetStatus (int state);
void kCpu_Statistics ();
int kSys_NewPid();
int kSys_NewIno();

// ---------------------------------------------------------------------------
extern kCpuCore_t kCPU;
extern kSysCore_t kSYS;

#endif /* KINFO_H__ */
