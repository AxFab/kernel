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
*      Implementation of various locks (blocking).
*/
#ifndef _SKC_SPLOCK_H
#define _SKC_SPLOCK_H 1

#include <smkos/atomic.h>

typedef struct splock splock_t; /* spin-lock, block-IRQ. */
typedef struct rowlock rowlock_t; /* read-write lock, block-IRQ, no-upgrade. */
typedef struct rwlock rwlock_t; /* read-write lock, upgradable. */


struct splock 
{
  atomic_t spin_;
  const char *at_;
  int cpu_;
};

struct rwlock
{
  splock_t spl_;
  atomic_t readers_;
};

// http://locklessinc.com/articles/locks/

/* Lock for write purpose */
void wri_lock(rowlock_t *lock);
void wri_unlock(rowlock_t *lock);
void wri_trylock(rowlock_t *lock);
int wri_locked(rowlock_t *lock);

void rdi_lock(rowlock_t *lock);
void rdi_unlock(rowlock_t *lock);
void rdi_trylock(rowlock_t *lock);
int rdi_locked(rowlock_t *lock);

void sp_lock(splock_t *lock);
void sp_unlock(splock_t *lock);
void sp_trylock(splock_t *lock);
int sp_locked(splock_t *lock);

void wr_lock(rwlock_t *lock);
void wr_unlock(rwlock_t *lock);
void wr_trylock(rwlock_t *lock);
int wr_locked(rwlock_t *lock);

void rd_lock(rwlock_t *lock);
void rd_unlock(rwlock_t *lock);
void rd_trylock(rwlock_t *lock);
void rd_upgrade(rwlock_t *lock);
int rd_locked(rwlock_t *lock);

void irq_on();
void irq_off();
int irq_check_on();

#include <skc/impl/splock.h>

#endif  /* _SKC_SPLOCK_H */

#if 0

#ifndef __si
# define __si
#endif

/* Lock for write purpose */
__si void sowr_lock(rowlock_t *l)
{
  unsigned int me = atomic_xadd(&l->u_, (1<<16));
  unsigned char val = me >> 16;
  while (val != l->s.write_) 
    cpu_relax();
}

__si void sowr_unlock(rowlock_t *l)
{
  rowlock_t t = *l;
  barrier();
  t.s.write_++;
  t.s.read_++;
  l->us_ = t.us_;
}

__si int sowr_trylock(rowlock_t *l)
{
  unsigned me = l->s.users;
  unsigned char menew = me + 1;
  unsigned read = l->s.read << 8;
  unsigned cmp = (me << 16) + read + me;
  unsigned cmpnew = (menew << 16) + read + me;

  if (cmpxchg(&l->u, cmp, cmpnew) == cmp) return 0;
  
  return EBUSY;
}

__si void rowlock_rdlock(rowlock *l)
{
  unsigned me = atomic_xadd(&l->u, (1<<16));
  unsigned char val = me >> 16;
  
  while (val != l->s.read) cpu_relax();
  l->s.read++;
}

__si void rowlock_rdunlock(rowlock *l)
{
  atomic_inc(&l->s.write);
}

__si int rowlock_rdtrylock(rowlock *l)
{
  unsigned me = l->s.users;
  unsigned write = l->s.write;
  unsigned char menew = me + 1;
  unsigned cmp = (me << 16) + (me << 8) + write;
  unsigned cmpnew = ((unsigned) menew << 16) + (menew << 8) + write;

  if (cmpxchg(&l->u, cmp, cmpnew) == cmp) return 0;
  
  return EBUSY;
}



union ticketlock
{
  unsigned u;
  struct
  {
    unsigned short ticket;
    unsigned short users;
  } s;
};

static void ticket_lock(ticketlock *t)
{
  unsigned short me = atomic_xadd(&t->s.users, 1);

  while (t->s.ticket != me) cpu_relax();
}

static void ticket_unlock(ticketlock *t)
{
  barrier();
  t->s.ticket++;
}

static int ticket_trylock(ticketlock *t)
{
  unsigned short me = t->s.users;
  unsigned short menew = me + 1;
  unsigned cmp = ((unsigned)me << 16) + me;
  unsigned cmpnew = ((unsigned)menew << 16) + me;

  if (cmpxchg(&t->u, cmp, cmpnew) == cmp) return 0;

  return EBUSY;
}

static int ticket_lockable(ticketlock *t)
{
  ticketlock u = *t;
  barrier();
  return (u.s.ticket == u.s.users);
}
#endif
