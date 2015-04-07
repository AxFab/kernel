/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Routines for page completions
 */
#include <kernel/core.h>
#include <kernel/scheduler.h>
#include <kernel/memory.h>
#include <kernel/vfs.h>
#include <kernel/mmu.h>


// ---------------------------------------------------------------------------
static int page_inode (kVma_t *vma, size_t address)
{
  size_t offset = vma->offset_ + ((size_t)address - vma->base_);
  int rights = 0;

  // @todo rights &= process->CAPACITIES & user->CAPACITIES!!
  if (vma->flags_ & VMA_WRITE) rights |= VMA_WRITE;

  if (vma->flags_ & VMA_READ) rights |= VMA_READ;

  if (vma->flags_ & VMA_EXEC /* && rights */) {
    // assert ();    todo check we got the rights
    rights |= VMA_EXEC;
  }

  // Search the page
  page_t page;

  if (inode_page(vma->ino_, offset, &page))
    return __geterrno();

  // Resolve
  if (vma->flags_ & VMA_SHARED) {
    mmu_resolve(address, page, rights, false);
  } else {
    // Should use copy on write
    // Mean set to read only and if not OK, then copy!
    page_t copy = mmu_newpage();
    mmu_resolve(address, copy, rights, false);
    void *src = mmu_temporary (&page);
    memcpy ((void *)address, src, PAGE_SIZE);
  }

  return __noerror();
}


// @todo thread_kill supposed that we are executing a task, which may not be true!
#define thread_kill(s) kpanic("PANIC, learn to kill process for SIGSEV at address [%x]\n",address);
// ---------------------------------------------------------------------------
int page_fault (size_t address, int cause)
{
  kAddSpace_t *mspace = NULL;

  if (kCPU.current_ != NULL)
    mspace = &kCPU.current_->process_->memSpace_;

  bool userspace = cause & PF_USER;
  assert ((cause & PF_PROT) == 0); // @todo handle this
  assert ((cause & PF_RSVD) == 0); // @todo handle this
  assert ((cause & PF_INSTR) == 0); // @todo handle this

  int space = mmu_addspace(address);

  if (space == AD_UNUSED) {
    if (!userspace)
      kpanic ("Kernel access invalid data at 0x%x\n", address);

    return thread_kill(SIGSEV);
  }

  if (space == AD_KERNEL) {
    if (userspace)
      return thread_kill(SIGSEV);

    return mmu_resolve (address, 0, VMA_KERNEL | VMA_READ | VMA_WRITE, true);
  }

  assert (space == AD_USERSP);
  assert (mspace != NULL);

  kVma_t *vma = addspace_find(mspace, address);

  if (vma == NULL)
    return thread_kill(SIGSEV);

  if (vma->flags_ & VMA_STACK)
    return mmu_resolve(address, 0, VMA_READ | VMA_WRITE, true);

  if (vma->flags_ & VMA_FILE) {
    if (vma->ino_ != NULL)
      return page_inode(vma, ALIGN_DW((size_t)address, PAGE_SIZE));

    return thread_kill(SIGSEV);
    // Fill inode !
  }

  assert (IS_PW2(vma->flags_ & (VMA_STACK | VMA_SHM)));
  return mmu_resolve(address, 0, vma->flags_ & VMA_MMU, true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
