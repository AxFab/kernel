#include <kernel/core.h>

// Kernel Info structures
kCpuCore_t kCPU;
kSysCore_t kSYS;

// kHdwCore_t kHDW = {
//   USR_SPACE_BASE,
//   USR_SPACE_LIMIT,
//   PG_BITMAP_ADD,
//   PG_BITMAP_LG,
//   (uint32_t*)0x2000, // directory mapping of the kernel
//   (uint32_t*)0x3000,
//   (uint32_t*)0x4000, // directory for screen buffer
// };


// ========== =================================================================
// ---------------------------------------------------------------------------
static int kSys_RunnableTasks()
{
  return kSYS.tasksCount_[(int)TASK_STATE_WAITING] +
      kSYS.tasksCount_[(int)TASK_STATE_EXECUTING];
}

// ---------------------------------------------------------------------------
int kSys_NewPid()
{
  return atomic_add_i32 (&kSYS.autoPid_, 1);
}

// ---------------------------------------------------------------------------
int kSys_NewIno()
{
  return atomic_add_i32 (&kSYS.autoIno_, 1);
}

// ===========================================================================

// ---------------------------------------------------------------------------
/** Change the status of the CPU
 *  note that status flow may be tricky on interruptable-interrupts
 *  FIXME Do task timing at the same time
 */
int kCpu_SetStatus (int state)
{
  assert (state > 0 && state < CPU_STATE_COUNT);

  int oldstate = kCPU.state_;
  nanotime_t now = time (NULL);
  nanotime_t elapsed = now - kCPU.lastStatus_;
  kCPU.stateTime_[kCPU.state_] += elapsed;
  kCPU.stateTime_[0] += elapsed;
  kCPU.lastStatus_ = now;
  kCPU.state_ = state;
  return oldstate;
}

// ---------------------------------------------------------------------------
void kCpu_Statistics ()
{
  int i;

  if (kCPU.cpuNo_ == 0) {
    for (i = 0; i < KRN_LOADAVG_COUNT; ++i) {
      float coef = kSYS.loadCoef_[i];
      kSYS.loadAvg_[i] =
          kSys_RunnableTasks() * coef + (1.0f - coef) * kSYS.loadAvg_[i];
    }
  }

  kCpu_SetStatus (kCPU.state_);

  nanotime_t elapsed = kCPU.stateTime_[0];
  for (i = 1; i < CPU_STATE_COUNT; ++i) {
    kCPU.statistics_[i] = (int)(kCPU.stateTime_[i] * KSTATS_PRECISION / elapsed);
  }

  memset (kCPU.stateTime_, 0, sizeof (kCPU.stateTime_));
}

int kcpu_state()
{
  return kCPU.state_;
}

