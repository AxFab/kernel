#pragma once
#include <smkos/spinlock.h>

/** @brief Common structure that hold information about a mutex. */
struct mutex {
  atomic_t key_;
  struct spinlock lock_;
};

/** @brief Common structure that hold information about a semaphore. */
struct semaphore {
  atomic_t value_;
  /* int flags_;*/
  struct spinlock lock_;
};


/* ----------------------------------------------------------------------- */
static inline int mtx_lock(struct mutex *mtx)
{
  if (mtx->key_ == 0 && atomic_xchg(&mtx->key_, 1) == 0)
    return __seterrno(0);

  /* klock(mtx->lock_);*/
  /* Register as waiting */
  return __seterrno(EAGAIN);
}


/* ----------------------------------------------------------------------- */
static inline int mtx_unlock(struct mutex *mtx)
{
  mtx->key_ = 0;
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
static inline void semaphore_aquire (struct semaphore *sem, int i)
{
  for (;;) {
    while (sem->value_ < i);

    klock (&sem->lock_);

    if (sem->value_ < i) {
      kunlock (&sem->lock_);
      continue;
    }

    atomic_add(&sem->value_, -i);
    kunlock (&sem->lock_);
    return;
  }
}


/* ----------------------------------------------------------------------- */
static inline bool semaphore_tryaquire (struct semaphore *sem, int i)
{
  if (sem->value_ < i)
    return false;

  klock (&sem->lock_);

  if (sem->value_ < i) {
    kunlock (&sem->lock_);
    return false;
  }

  atomic_add(&sem->value_, -i);
  kunlock (&sem->lock_);
  return true;
}


/* ----------------------------------------------------------------------- */
static inline void semaphore_release (struct semaphore *sem, int i)
{
  atomic_add(&sem->value_, i);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
