/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Yield scheduler algorithm.
 */
#include <kernel/scheduler.h>
#include <kernel/info.h>
#include <kernel/memory.h>
#include <kernel/async.h>

// ---------------------------------------------------------------------------
static int ksch_timeslice (kTask_t* task)
{
#ifndef __KERNEL
  return 1;
#else
  ltime_t elapsed = ltime(NULL) - task->execStart_;
  if (elapsed == 0) elapsed = 1;
  long weight = (21 - task->niceValue_) * 2;
  long weightElapsed = (task->elapsedUser_ * kSYS.schedLatency_) / elapsed / 1;//kSYS.cpuCount_;
  long weightProc = (weight * kSYS.schedLatency_) / kSYS.prioWeight_;
  long sliceMicro = weightProc - weightElapsed;
  if (sliceMicro < kSYS.minTimeSlice_)
      return kSYS.minTimeSlice_ / 1000;
  return (int)sliceMicro / 1000;
#endif
}


// ---------------------------------------------------------------------------
void ksch_ticks (kCpuRegs_t* regs)
{
  async_ticks();
  if (!ksch_ontask()) {
    ksch_pick ();

  } else if (kCPU.current_->state_ == TASK_STATE_ABORTING) {
    ksch_stop (TASK_STATE_ZOMBIE, regs);
    ksch_pick ();

  } else if (--kCPU.current_->timeSlice_ > 0) {
    assert (kCPU.current_->state_ == TASK_STATE_EXECUTING);
    kCpu_SetStatus (CPU_STATE_USER);

  } else if (kSYS.tasksCount_[TASK_STATE_WAITING] <= 0) {
    assert (kCPU.current_->state_ == TASK_STATE_EXECUTING);
    kCPU.current_->timeSlice_ = ksch_timeslice (kCPU.current_);

  } else {
    ksch_stop (TASK_STATE_WAITING, regs);
    ksch_pick ();
  }
}


// ---------------------------------------------------------------------------
void ksch_pick ()
{
  kTask_t* pick;
  assert (!ksch_ontask());

  cli();
  int waiting = atomic_sub_i32 (&kSYS.tasksCount_[TASK_STATE_WAITING], 1);
  if (waiting >= 0) {

    pick = kCPU.current_->nextSc_;
    do {
      if (pick->state_ != TASK_STATE_WAITING) {
        pick = pick->nextSc_;
        continue;
      }

      if (!ktrylock(&pick->lock_, LOCK_TASK_NEXT)) {
        pick = pick->nextSc_;
        continue;
      }

      if (pick->state_ != TASK_STATE_WAITING) { // FIXME unlikely()
        kunlock (&pick->lock_);
        pick = pick->nextSc_;
        continue;
      }

      // Run the task
      kCPU.current_ = pick;
      pick->lastWakeUp_ = ltime(NULL);
      pick->timeSlice_ = ksch_timeslice (pick);
      pick->state_ = TASK_STATE_EXECUTING;
      atomic_inc_i32 (&kSYS.tasksCount_[pick->state_]);
      pick->execOnCpu_ = kCPU.cpuNo_;
      kCpu_SetStatus (CPU_STATE_USER);
      kunlock (&pick->lock_);

      if (KLOG_SCH) kprintf ("scheduler] calling switch <%x, %x> [%x]\n", &pick->regs_, &pick->process_->dir_, pick->kstack_ + PAGE_SIZE * 2 - 0x10);

      kpg_reset_stack ();
      kCpu_Switch (&pick->regs_, &pick->process_->dir_, pick->kstack_ + PAGE_SIZE * 2 - 0x10);
      return;

    } while (pick != kCPU.current_);

    kprintf ("scheduler] pick next didn't find any available task\n");
  }

  if (KLOG_SCH) kprintf ("scheduler] No task go idle...\n");
  atomic_inc_i32 (&kSYS.tasksCount_[TASK_STATE_WAITING]);
  kCpu_SetStatus (CPU_STATE_IDLE);
  // In case the current task is on garbadge collector
  kCPU.current_ = kCPU.current_->nextSc_;
  // FIXME call __asm__ HLT
  task_print ();
  kpanic ("HLT is not implemented.\n");
}

