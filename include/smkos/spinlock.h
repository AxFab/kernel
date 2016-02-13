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
 *      Implementation of a spinlock.
 */
#ifndef _NO_SKC
#include <skc/splock.h>
#else
#pragma once
#include <smkos/kernel.h>
#include <smkos/atomic.h>

struct spinlock {
  int cpu_;
  atomic_t key_;
  const char *where_;
  struct spinlock* next_;
};
#define INIT_SPINLOCK { 0, 0, NULL, NULL }

extern struct spinlock *SP_first, *SP_last;

#define klock(l)         klock_(l,__AT__);
#define kunlock(l)       kunlock_(l);

#define LockCounter  (*__lockcounter())
int *__lockcounter();

/* ----------------------------------------------------------------------- */
static inline void klock_(struct spinlock *locker, const char *where)
{
  /* int t = 100000000;*/
  for (;;) {
    while (locker->key_ != 0) {
      /* if (--t == 0)
         kpanic("Stuck at %s by %s\n", where, locker->where_); */
      cpause();
    }

    cli();

    if (atomic_xchg(&locker->key_, 1) == 0) {
      if (SP_first)
        SP_last->next_ = locker;
      else
        SP_first = locker;
      locker->next_ = NULL;
      SP_last = locker;

      ++LockCounter;
      locker->cpu_ = kCpuNo;
      locker->where_ = where;
      return;
    }
  }
}


/* ----------------------------------------------------------------------- */
static inline void kunlock_(struct spinlock *locker)
{
  struct spinlock *list = NULL;

  assert (locker->key_ == 1);
  assert (locker->cpu_ == kCpuNo);

  locker->key_ = 0;
  locker->cpu_ = 0;
  locker->where_ = NULL;

  if (SP_first == locker) {
    SP_first = locker->next_;
  }
  else {
    list = SP_first;
    while (list->next_ != locker)
      list = list->next_;
    list->next_ = locker->next_;
  }

  if (SP_last == locker)
    SP_last = list;

  if (--LockCounter == 0 && NULL)
    sti();
}


/* ----------------------------------------------------------------------- */
static inline int kislocked(struct spinlock *locker)
{
  return locker->key_ != 0;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
#endif