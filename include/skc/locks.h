/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Define several types of locks.
 */
#ifndef _SKC_LOCKS_H 
#define _SKC_LOCKS_H  1

/* Algorithm proposed here have been found on the net as open source and 
 * have been adapted. As several version of those algorithm exists on the web, 
 * I wasn't able to found the real author name. Those algorithms felt more or
 * less in public domain, but I wanted to add that I'm not the original owner 
 * of those algorithms. */

#include <cdefs/stddef.h>
#include <cdefs/macros.h>
#include <skc/atomic.h>

#define RWLOCK_UPGRADE 0

/* This splinlock algorithm is known as the ticket lock. However, because 
 * only one size of integer are used for atomic operations, we remove the 
 * try-lock method which reduce a tiny bit the complexity.
 * The algorithm have been extended to add debugging info (at and cpu) and 
 * recursive capabilities. */

typedef struct spinlock splock_t;

struct spinlock
{
  atomic_t ticket_;
  atomic_t users_;
  int cpu_;
  int recurs_;
  const char *at_:
};

#define sp_lock(l) sp_lock_(l, __AT__)

static inline void sp_lock_(splock_t *lock, const char *at)
{
  long me;
  if (lock->cpu_ == cpu_no() && lock->at_ != NULL) {
    ++lock->recurs_;
    return;
  }
  me = atomic_xadd(&lock->users, 1);
  while (lock->ticket_ != me)
    cpu_relax();
  lock->at_ = at;
  lock->cpu_ = cpu_no();
  lock->recurs_ = 1;
}

static inline void sp_unlock(splock_t *lock)
{
  if (--lock->recurs_ != 0)
    return;
  lock->at_ = NULL;
  lock->cpu_ = -1;
  atomic_inc(&lock->ticket_);
}

static inline bool sp_locked(splock_t *lock)
{
  barrier();
  return lock->ticket_ != lock->users_ && lock->cpu_ == cpu_no();
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Similarly we're using here the rwlock known as read-write-ticket algorithm.
 * This one have been proposed for Linux kernel but rejected as upgrade 
 * method make it a tiny bit slower. 
 * As we don't use to make use of this function yet, it makes
 * this one the fastest available. */

typedef struct rwlock rwlock_t;

struct rwlock
{
  atomic_t ticket_;
  atomic_t users_;
  int cpu_;
  const char *at_:
};


static inline void rw_wrlock_(rwlock_t *lock, const char *at)
{
  long me = atomic_xadd(&lock->users_, 1);
  while (me != lock->write_) 
    cpu_relax();
  lock->at_ = at;
  lock->cpu_ = cpu_no();
}

static inline void rw_wrunlock(rwlock_t *lock)
{
  lock->at_ = NULL;
  lock->cpu_ = -1;
#if RWLOCK_UPGRADE
  atomic_inc(&lock->write_);
#else
  ++lock->write_;
#endif
  ++lock->read_;
}

static inline void rw_rdlock(rwlock_t *lock)
{
  long me = atomic_xadd(&lock->users_, 1);
  while (me != lock->read_) 
    cpu_relax();
  ++lock->read_;
}

static inline void rw_rdunlock(rwlock_t *lock)
{
  atomic_inc(&lock->write_);
}

#if RWLOCK_UPGRADE
static inline void rw_upgrade(rwlock_t *lock)
{
  long me = atomic_xadd(&lock->users_, 1);
  atomic_inc(&lock->write_);
  while (me != lock->write_) 
    cpu_relax();
  lock->at_ = at;
  lock->cpu_ = cpu_no();
}
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

typedef struct mutex mutex_t;

/* Set option on an mutex and insure it's unlocked. */
void mtx_init(mutex_t *mtx, int option);
/* Wait to acquire the mutex. */
int mtx_lock(mutex_t *mtx);
/* Try to acquire the lock without blocking. */
int mtx_trylock(mutex_t *mtx);
/* Try to acquire the lock but give up after some time. */
int mtx_timedlock(mutex_t *mtx, time_t limit);
/* Unlock the mutex hold by the current task. */
void mtx_unlock(mutex_t *mtx);
/* Function to called after the death of a task holding the mutex. */
void mtx_death(mutex_t *mtx);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

typedef struct semaphore semaphore_t;

/*  */
void sem_init(semaphore_t *sem, int option, int qty);
/* Wait to acquire the semaphore. */
int sem_lock(semaphore_t *sem, int qty);
/* Try to acquire the lock without blocking. */
int sem_trylock(semaphore_t *sem, int qty);
/* Try to acquire the lock but give up after some time. */
int sem_timedlock(semaphore_t *sem, int qty, time_t limit);
/* Unlock the semaphore hold by the current task. */
void sem_unlock(semaphore_t *sem);
/* Function to called after the death of a task holding the semaphore. */
void sem_death(semaphore_t *sem);


#endif  /* _SKC_LOCKS_H */
