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
#include <skc/locks.h>
#include <errno.h>
#include <string.h>

task_t *tsk_current();
void tsk_wait(mutex_t *mtx, long time);



/* Set option on an mutex and insure it's unlocked. */
void mtx_init(mutex_t *mtx, int option) 
{
  memset(mtx, 0, sizeof(*mtx));
  mtx->flags_ = option;
}

/* Wait to acquire the mutex. */
int mtx_lock(mutex_t *mtx) 
{
  if (mtx->locked_ && mtx->task_ == tsk_current()) {
    // TODO -- What if the lock is release without action of the task (timed)
    if (mtx->flags_ & MTX_RECURSIVE) {
      ++mtx->locked_;
      return 0;
    }

    return EDEADLOCK; // Recursive is not allowed
  }

  for (;;) {
    if (mtx->locked_)
      tsk_wait(mtx, -1);
    if (mtx->error_)
      return mtx->error_;
    if (atomic_xchg(&mtx->locked_, 1) == 0)
      break;
  }
  mtx->task_ = tsk_current();
  return atomic_xchg(&mtx->warn_, 0);
}

// /* Try to acquire the lock without blocking. */
// int mtx_trylock(mutex_t *mtx, int option) 
// {
//   if (mtx->locked_ && mtx->task_ == tsk_current()) {
//     // TODO -- What if the lock is release without action of the task (timed)
//     if (mtx->flags_ & MTX_RECURSIVE) {
//       ++mtx->locked_;
//       return 0;
//     }

//     return MTX_DEADLOCK; // Recursive is not allowed
//   }

//   if (atomic_xchg(mtx->locked_, 1) != 0)
//     return MTX_AGAIN;
//   mtx->task_ = tsk_current();
//   return atomic_xchg(mtx->warn_, 0);
// }

// /* Try to acquire the lock but give up after some time. */
// int mtx_timedlock(mutex_t *mtx, time_t limit) 
// {
//   time_t now = time(NULL);
//   if (mtx->locked_ && mtx->task_ == tsk_current()) {
//     // TODO -- What if the lock is release without action of the task (timed)
//     if (mtx->flags_ & MTX_RECURSIVE) {
//       ++mtx->locked_;
//       return 0;
//     }

//     return MTX_DEADLOCK; // Recursive is not allowed
//   }

//   for (;;) {
//     if (mtx->locked_) 
//       tsk_wait(mtx, -1);
//     if (mtx->error_)
//       return mtx->error_;
//     now = time(NULL);
//     if (now > limit)
//       return MTX_TIMEOUT;
//     if (atomic_xchg(mtx->locked_, 1) == 0)
//       break;
//   }
//   mtx->task_ = tsk_current();
//   return atomic_xchg(mtx->warn_, 0);
// }

/* Unlock the mutex hold by the current task. */
void mtx_unlock(mutex_t *mtx) 
{
  mtx->error_ = 0;
  mtx->warn_ = 0;
  mtx->task_ = NULL;
  barrier();
  mtx->locked_ = 0;

}

// /* Function to called after the death of a task holding the mutex. */
// void mtx_death(mutex_t *mtx, int option) 
// {
//   if ()
//   mtx->
// }


task_t *tsk_current() 
{
  return NULL;
}

void tsk_wait(mutex_t *mtx, long time)
{

}
