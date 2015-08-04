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
#include <smkos/kapi.h>
#include <smkos/alloc.h>
#include <smkos/kstruct/task.h>

#include <stdlib.h>


/* ----------------------------------------------------------------------- */
page_t mmu_newdir()
{
  return 0x8000;
}


/* ----------------------------------------------------------------------- */
int mmu_resolve (size_t address, page_t page, int access, bool zero)
{
  __unused(address);
  __unused(page);
  __unused(access);
  __unused(zero);
  return 0;
}

void mmu_clean_page(size_t address)
{
  memset((void *)address, 0, 0x1000);
}

/* ----------------------------------------------------------------------- */
page_t mmu_newpage()
{
  assert (kSYS.pageAvailable_ > 0);
  atomic_dec(&kSYS.pageAvailable_);
  atomic_inc(&kSYS.pageUsed_);
  return kSYS.pageUsed_ * 0x1000 + 0x100000;
}

/* ----------------------------------------------------------------------- */
void mmu_releasepage(page_t page)
{
  __unused(page);
  atomic_inc(&kSYS.pageAvailable_);
  atomic_dec(&kSYS.pageUsed_);
}

/* ----------------------------------------------------------------------- */
void mmu_load_env()
{
  void *ptr = valloc_(8 * _Mb_);
  // alloc_init((size_t)malloc(2 * _Mb_), 2 * _Mb_);
  kSYS.mspace_ = KALLOC(kMemSpace_t);
  kSYS.scheduler_ = KALLOC(kScheduler_t);
  memset (ptr, 0, 8 * _Mb_);
  area_init(kSYS.mspace_, (size_t)ptr, 8 * _Mb_);
}

/* ----------------------------------------------------------------------- */
void mmu_leave_env()
{
  // @todo Assert mspace and scheduler are empty
  vfree((void *)kSYS.mspace_->base_);
  kfree(kSYS.mspace_);
  kfree(kSYS.scheduler_);
}


/* ----------------------------------------------------------------------- */
void mmu_map_userspace(kMemSpace_t *sp)
{
  void *ptr = valloc_(8 * _Mb_);
  memset (ptr, 0, 8 * _Mb_);
  area_init(sp, (size_t)ptr, 8 * _Mb_);
}

void mmu_destroy_userspace(kMemSpace_t *sp)
{
  vfree((void *)sp->base_);
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
