#include <kinfo.h>
#include <kcpu.h>

/*
 * The current thread has a counter of kernel lock acquiered.
 * Until this counter is positive, the thread block interruptions.
 * When the counter fall back to zero, interruption are restored.
 * The counter must reach zero before starting any asynchrones jobs.
 */

void klock (spinlock_t* lock, int why)
{
  while (atomic_xchg_i32 (&lock->key_, 1) != 0);

  atomic_inc_i32 (&kCPU.lockCounter);
  lock->cpu_ = kCPU.cpuNo_;
  lock->why_ = why;
}

int ktrylock (spinlock_t* lock, int why)
{
  if (atomic_xchg_i32 (&lock->key_, 1) != 0)
    return !0;

  lock->cpu_ = kCPU.cpuNo_;
  lock->why_ = why;
  atomic_inc_i32 (&kCPU.lockCounter);
  return 0;
}

void kunlock (spinlock_t* lock)
{
  assert (lock->key_ == 1);
  assert (lock->cpu_ == kCPU.cpuNo_);
  lock->key_ = 0;
  lock->cpu_ = 0;
  lock->why_ = 0;
  atomic_dec_i32 (&kCPU.lockCounter);
}

int kislocked (spinlock_t* lock)
{
  return lock->key_ != 0 && lock->cpu_ == kCPU.cpuNo_;
}

int klockcount ()
{
  return kCPU.lockCounter;
}

