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
 *      Page fault handling.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <signal.h>


#define AD_USERSP 1
#define AD_UNUSED -1
#define AD_KERNEL 0



/* ----------------------------------------------------------------------- */
static int page_rights(int flags)
{
  int rights = 0;

  /// @todo rights &= process->CAPACITIES & user->CAPACITIES!!
  if (flags & VMA_WRITE)
    rights |= VMA_WRITE;

  if (flags & VMA_READ)
    rights |= VMA_READ;

  if (flags & VMA_EXEC /* && rights */)
    rights |= VMA_EXEC;

  return rights;
}


/* ----------------------------------------------------------------------- */
static int page_inode (kMemArea_t *area, size_t address)
{
  int err;
  page_t phys;
  int rights = page_rights(area->flags_);
  size_t offset = area->offset_ + ((size_t)address - area->address_);
  kPage_t *page;

  // Search the page on cache.
  klock (&area->ino_->lock_);
  page = bb_search(&area->ino_->pageTree_, offset, kPage_t, treeNd_);

  if (page == NULL) {
    if (open_fs(area->ino_))
      return __geterrno();

    if (area->ino_->dev_->fs_->map) {
      err = area->ino_->dev_->fs_->map(area->ino_, offset, &phys);

      if (!err)
        err = mmu_resolve(address, phys, rights, false);

    } else if (area->ino_->dev_->fs_->read) {
      phys = mmu_newpage();
      err = mmu_resolve(address, phys, rights, false);

      if (!err)
        err = area->ino_->dev_->fs_->read(area->ino_, (void *)address, PAGE_SIZE, offset);

    } else {
      err = EIO;
    }

    close_fs (area->ino_);

    if (err)
      return __seterrno(err);

    // Store the new page on cache.
    page = KALLOC(kPage_t);
    page->offset_ = offset;
    page->phys_ = phys;
    page->treeNd_.value_ = offset;
    bb_insert (&area->ino_->pageTree_, &page->treeNd_);

  } else {
    err = mmu_resolve(address, page->phys_, rights, false);
    kunlock (&area->ino_->lock_);
  }

  /// @todo Should use copy on write
  return __seterrno(err);
}

int PFp = 0;
/* ----------------------------------------------------------------------- */
int page_fault (size_t address, int cause)
{
  int mtype;
  kMemSpace_t *mspace = kSYS.mspace_;
  kMemArea_t *area = NULL;
  bool userspace = cause & PF_USER;

  // if (!(cause & PF_KERN)) {
  // if (address < 0xd0000000) {
  //   kprintf("PF] %x (%d)\n", address, cause);
  //   kstacktrace();
  //   for(;;);
  // }
  // }

  if (address < mspace->base_ || address >= mspace->limit_) {
    if (kCPU.current_ != NULL) {
      mspace = &kCPU.current_->process_->mspace_;
      // PFp = 1;
    }

    if (address < mspace->base_ || address >= mspace->limit_)
      return sched_signal(SIGSEGV, address, __AT__);
  } else if (userspace)
    return sched_signal(SIGSEGV, address, __AT__);

  assert(kCPU.lockCounter_ == 0);
  assert(mspace != NULL);
  area = area_find(mspace, address);
  mtype = area->flags_ & VMA_TYPE;
  assert (POW2(mtype));

  if (area == NULL)
    return sched_signal(SIGSEGV, address, __AT__);

  switch (mtype) {
  case VMA_HEAP:
  case VMA_STACK:
    assert (mspace != kSYS.mspace_); /* Never on kernel */
    if (PFp) kprintf("\033[96m/\033[33m0x%0x.H\033[96m/\033[0m", address);
    return mmu_resolve(address, 0, VMA_READ | VMA_WRITE, true);

  case VMA_FIFO:
    assert (mspace == kSYS.mspace_); /* Pipe can only be mounted on kernel space */
    if (PFp) kprintf("\033[96m/\033[33m0x%0x.Q\033[96m/\033[0m", address);  
    return mmu_resolve(address, 0, VMA_READ | VMA_WRITE | VMA_KERNEL, true);

  case VMA_FILE:
    assert (area->ino_ != NULL);
    if (PFp) kprintf("\033[96m/\033[33m0x%0x.F\033[96m/\033[0m", address);  
    return page_inode(area, ALIGN_DW((size_t)address, PAGE_SIZE));

  case VMA_SHM:
    assert (((area->flags_ & VMA_KERNEL) != 0) == (mspace == kSYS.mspace_));
    if (PFp) kprintf("\033[96m/\033[33m0x%0x.S\033[96m/\033[0m", address);  
    return mmu_resolve(address, 0, area->flags_ & VMA_MMU, true);

  default:
    kpanic("Error on page fault handler\n");
    return ENOSYS;
  }
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
