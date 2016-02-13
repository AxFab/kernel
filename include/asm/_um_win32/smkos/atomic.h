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
 *      Atomical operations on an integer value.
 */
#pragma once

typedef int atomic_t;


static __inline void atomic_inc(volatile atomic_t* ref)
{
  /*__asm {
    lock inc dword [ref]
  }*/
  ++(*ref);
}


static __inline void atomic_dec(volatile atomic_t* ref)
{
  --(*ref);
}

static __inline void atomic_set(volatile atomic_t* ref, atomic_t val)
{
  *ref = val;
}

static __inline atomic_t atomic_read(volatile atomic_t* ref)
{
  return *ref;
}


static __inline atomic_t atomic_xchg(volatile atomic_t* ref, atomic_t val)
{
  register atomic_t rval = *ref;
  *ref = val;
  return rval;
}


static __inline atomic_t atomic_add(volatile atomic_t *ref, atomic_t val)
{
  *ref += val;
  return *ref;
}

static __inline atomic_t atomic_sub(volatile atomic_t *ref, atomic_t val)
{
  *ref -= val;
  return *ref;
}


/* ----------------------------------------------------------------------- */
int cpu_no();
void cpu_irq_on();
void cpu_irq_off();
void cpu_relax();
static __inline void cli() { /*asm volatile("cli");*/ }
static __inline void sti() { /*asm volatile("sti");*/ }
static __inline void cpause() 
{
  __asm { 
    pause 
  } 
}

