#ifndef KERNEL_SPINLOCK_H__
#define KERNEL_SPINLOCK_H__

#include <atomic.h>
#include <kernel/core.h>
#include <kernel/info.h>

/*
 * The current thread/CPU has a counter of kernel lock acquiered.
 * Until this counter is positive, the thread block interruptions.
 * When the counter fall back to zero, interruption are restored.
 * The counter must reach zero before starting any asynchrones jobs.
 */

#define klock(l,...)  sl_lock_(l,__AT__)
static inline void sl_lock_ (spinlock_t* lock, const char* where)
{
  int t = 100000000;
  while (atomic_xchg_i32(&lock->key_, 1) != 0) {
    if (!t--) {
      kprintf("Stuck at %s by %s\n", where, lock->where_);
      // kstacktrace(5);
      t = 100000000;
    }
  };

  cli();
  atomic_inc_i32(&kCPU.lockCounter);
  lock->cpu_ = kCPU.cpuNo_;
  lock->where_ = where;
}


#define ktrylock(l,...)  sl_trylock_(l,__AT__)
static inline int sl_trylock_ (spinlock_t* lock, const char* where)
{
  if (atomic_xchg_i32 (&lock->key_, 1) != 0)
    return 0;

  lock->cpu_ = kCPU.cpuNo_;
  lock->where_ = where;
  cli ();
  atomic_inc_i32(&kCPU.lockCounter);
  return 1;
}

// static inline void sl_unlock (spinlock_t* lock)
static inline void kunlock (spinlock_t* lock)
{
  assert (lock->key_ == 1);
  assert (lock->cpu_ == kCPU.cpuNo_);
  lock->key_ = 0;
  lock->cpu_ = 0;
  lock->where_ = NULL;
  int locks = atomic_add_i32(&kCPU.lockCounter, -1);
  if (locks <= 0 && kCPU.ready_) 
    sti();
}

static inline int kislocked (spinlock_t* lock)
{
  return lock->key_ != 0 && lock->cpu_ == kCPU.cpuNo_;
}

// static inline int sl_count ()
static inline int klockcount ()
{
  return kCPU.lockCounter;
}


#endif /* KERNEL_SPINLOCK_H__ */
