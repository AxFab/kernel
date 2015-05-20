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
 *      Intel x86 MMU wrapper implementation.
 */
#include <smkos/kernel.h>
#include <smkos/arch.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/alloc.h>
#include "mmu.h"

/* ----------------------------------------------------------------------- */
/** Allocat a single page for the system and return it's physical address */
page_t mmu_newpage()
{
  static uint8_t *bitmap = (uint8_t *)PG_BITMAP_ADD;
  int i = 0, j = 0;

  while (i < PG_BITMAP_LG && bitmap[i] == 0xFF)
    i++;

  if (i >= PG_BITMAP_LG)
    kpanic ("Not a single page available\n");

  uint8_t value = bitmap[i];

  while (value & 1) {
    ++j;
    value = value >> 1;
  }

  bitmap[i] = bitmap[i] | (1 << j);
  --kSYS.pageAvailable_;
  return (page_t)(uint32_t)((i * 8 + j) * PAGE_SIZE);
}



/* ----------------------------------------------------------------------- */
/** Mark a single physique page, returned by mmu_newpage, as available again */
void mmu_releasepage(page_t page)
{
  static uint8_t *bitmap = (uint8_t *)PG_BITMAP_ADD;
  int i = (page / PAGE_SIZE) / 8;
  int j = (page / PAGE_SIZE) % 8;

  if (i >= PG_BITMAP_LG || (bitmap[i] & (1 << j)) == 0)
    kpanic ("Release page with wrong args\n");

  bitmap[i] = bitmap[i] & (~(1 << j));
}


/* ----------------------------------------------------------------------- */
/** Remove the page from the context, eventualy memzero it first */
void mmu_clean_page(size_t address)
{
}

/* ----------------------------------------------------------------------- */
int mmu_resolve (size_t address, page_t page, int access, bool zero)
{
  assert (zero || page != 0);
  // assert (!zero || page == 0);

  int dir = (address >> 22) & 0x3ff;
  int tbl = (address >> 12) & 0x3ff;

  // kprintf ("resolve page fault at _[%d][%d] for %x - %x\n", dir, tbl, address, add);
  uint32_t *table = (access & VMA_KERNEL) ? MMU_LEVEL1_KRN() : MMU_LEVEL1();

  if (table[dir] == 0) {
    page_t dirPage = mmu_newpage();
    dirPage |= (MMU_ACCESS_WR | ((access & VMA_KERNEL) ? 0 : MMU_ACCESS_UR));
    table[dir] = dirPage;

    if (access & VMA_KERNEL)
      MMU_LEVEL1()[dir] = dirPage;

    memset(MMU_LEVEL2(dir), 0, PAGE_SIZE);
  }

  if (MMU_LEVEL2(dir)[tbl] == 0) {
    if (page == 0)
      page = mmu_newpage();

    page |= (access & VMA_WRITE) ? MMU_ACCESS_WR : 0;
    page |= (access & VMA_KERNEL) ? 0 : MMU_ACCESS_UR;
    MMU_LEVEL2(dir)[tbl] = page;

    if (zero)
      memset((void *)(address & ~(PAGE_SIZE - 1)), 0, PAGE_SIZE);
  }

  return __seterrno(0);
}



/* ----------------------------------------------------------------------- */
page_t mmu_newdir()
{
  int idx = (MMU_USERSP_LIMIT >> 22) & 0x3ff;
  page_t *kernelDir = (page_t *)MMU_PREALLOC_DIR;
  page_t *table = (page_t *)MMU_PREALLOC_TBL;
  page_t *dir = (page_t *)MMU_PREALLOC_NEW;
  page_t page = mmu_newpage();

  assert(kCPU.current_ != NULL);
  assert(kislocked(&kCPU.current_->process_->lock_));

  // Map a page to a temporary address
  ++kCPU.current_->process_->pagePrivate_;
  table[MMU_PREALLOC_NEW / PAGE_SIZE] = page | MMU_ACCESS_WR;

  // Fill the new directory
  memset(dir, 0, PAGE_SIZE);
  dir[0] = MMU_PREALLOC_TBL | MMU_ACCESS_WR; // Kernel first Mb
  dir[1023] = page | MMU_ACCESS_WR; // Mirror

  // Copy kernel heap -- not mandatory but avoid useless soft-pagefaults.
  for (; idx < 1022; ++idx)
    dir[idx] = kernelDir[idx];

  // No need to inval TBL, we swap dir right now
  table[MMU_PREALLOC_NEW / PAGE_SIZE] = 0;
  return page;
}


/* ----------------------------------------------------------------------- */
void mmu_load_env()
{
  memset ((void *)_Mb_, 0, _Mb_);
  alloc_init(_Mb_, _Mb_);
  kSYS.mspace_ = KALLOC(kMemSpace_t);
  kSYS.scheduler_ = KALLOC(kScheduler_t);
  area_init (kSYS.mspace_, (size_t)MMU_KHEAP_BASE, MMU_KHEAP_LIMIT - MMU_KHEAP_BASE);
}

/* ----------------------------------------------------------------------- */
void mmu_leave_env()
{
  // alloc_reset()
  kfree(kSYS.mspace_);
  kfree(kSYS.scheduler_);
}


void mmu_destroy_userspace(kMemSpace_t *sp)
{
  __unused(sp);
}


/* ----------------------------------------------------------------------- */
void mmu_map_userspace(kMemSpace_t *sp)
{
  area_init(sp, (size_t)MMU_USERSP_BASE, MMU_USERSP_LIMIT - MMU_USERSP_BASE);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
