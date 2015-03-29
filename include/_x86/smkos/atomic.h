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


/* ----------------------------------------------------------------------- */
static inline void atomic_inc (volatile atomic_t* ref)
{
  asm volatile ("lock incl %0" : "=m"(*ref));
}


/* ----------------------------------------------------------------------- */
static inline void atomic_dec (volatile atomic_t* ref)
{
  asm volatile ("lock decl %0" : "=m"(*ref));
}


/* ----------------------------------------------------------------------- */
static inline atomic_t atomic_xchg (volatile atomic_t* ref, atomic_t val)
{
  register atomic_t rval = val;
  asm volatile ( "lock xchg %1,%0" : "=m" (*ref), "=r" (rval) : "1" (val));
  return rval;
}


/* ----------------------------------------------------------------------- */
static inline atomic_t atomic_add(volatile atomic_t *ref, atomic_t val)
{
  asm volatile("lock xaddl %%eax, %2;"
               :"=a" (val) :"a" (val), "m" (*ref) :"memory");
  return val;
}


/* ----------------------------------------------------------------------- */
static inline void cli() { asm volatile("cli"); }
static inline void sti() { asm volatile("sti"); }
static inline void pause() { asm volatile("pause"); }

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
