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
 *      Usermode MMU wrapper implementation.
 */
#include <smkos/kernel.h>
#include <smkos/core.h>

#include <stdlib.h>


/* ----------------------------------------------------------------------- */
page_t mmu_newdir()
{
  return 0x8000;
}


/* ----------------------------------------------------------------------- */
int mmu_resolve (size_t address, page_t page, int access, bool zero)
{
  return 0;
}

/* ----------------------------------------------------------------------- */
page_t mmu_newpage()
{
  atomic_dec(&kSYS.pageAvailable_);
  atomic_inc(&kSYS.pageUsed_);
  return kSYS.pageUsed_ * 0x1000 + 0x100000;
}

/* ----------------------------------------------------------------------- */
void mmu_load_env()
{
  alloc_init((size_t)malloc(2 * _Mb_), 2 * _Mb_);
  kSYS.mspace_ = KALLOC(kMemSpace_t);
  kSYS.scheduler_ = KALLOC(kScheduler_t);
  area_init (kSYS.mspace_, (size_t)malloc(8 * _Mb_), 8 * _Mb_);
}


/* ----------------------------------------------------------------------- */
void mmu_map_userspace(kMemSpace_t *sp)
{
  area_init(sp, (size_t)malloc(8 * _Mb_), 8 * _Mb_);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
