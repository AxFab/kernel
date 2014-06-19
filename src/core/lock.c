#include <kinfo.h>
#include <kcpu.h>

/*
 * The current thread has a counter of kernel lock acquiered.
 * Until this counter is positive, the thread block interruptions.
 * When the counter fall back to zero, interruption are restored.
 * The counter must reach zero before starting any asynchrones jobs.
 */

void klock (spinlock_t* lock)
{
  while (_xchg_i32 (&lock->key_, 1) != 0);

  _inc_i32 (&kCPU.lockCounter);
  lock->tid_ = THREAD_ID;
}

int ktrylock (spinlock_t* lock)
{
  if (_xchg_i32 (&lock->key_, 1) != 0)
    return !0;

  lock->tid_ = THREAD_ID;
  _inc_i32 (&kCPU.lockCounter);
  return 0;
}

void kunlock (spinlock_t* lock)
{
  assert (lock->key_ == 1);
  assert (lock->tid_ == THREAD_ID);
  lock->key_ = 0;
  lock->tid_ = 0;
  _dec_i32 (&kCPU.lockCounter);
}

int kislocked (spinlock_t* lock)
{
  return lock->key_ != 0 && lock->tid_ == THREAD_ID;
}

int klockcount ()
{
  return kCPU.lockCounter;
}

