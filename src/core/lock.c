#include <kernel/info.h>
#include <kernel/cpu.h>

/*
 * The current thread has a counter of kernel lock acquiered.
 * Until this counter is positive, the thread block interruptions.
 * When the counter fall back to zero, interruption are restored.
 * The counter must reach zero before starting any asynchrones jobs.
 */
#undef klock
void klock (spinlock_t* lock, const char* where)
{
  int t = 100000000;
  while (atomic_xchg_i32 (&lock->key_, 1) != 0) {
    if (!t--) {
      kprintf("Stuck at %s by %s\n", where, lock->where_);
      kstacktrace (5);
      t = 100000000;
    }
  };

  atomic_inc_i32 (&kCPU.lockCounter);
  lock->cpu_ = kCPU.cpuNo_;
  lock->where_ = where;
}

#undef ktrylock
int ktrylock (spinlock_t* lock, const char* where)
{
  if (atomic_xchg_i32 (&lock->key_, 1) != 0)
    return 0;

  lock->cpu_ = kCPU.cpuNo_;
  lock->where_ = where;
  atomic_inc_i32 (&kCPU.lockCounter);
  return 1;
}

void kunlock (spinlock_t* lock)
{
  assert (lock->key_ == 1);
  assert (lock->cpu_ == kCPU.cpuNo_);
  lock->key_ = 0;
  lock->cpu_ = 0;
  lock->where_ = NULL;
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

