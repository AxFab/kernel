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
static int page_inode (kVma_t* vma, void* address)
{
  size_t offset = vma->offset_ + ((size_t)address - vma->base_);
  int rights = 0;
  // @todo rights &= process->CAPACITIES & user->CAPACITIES!!
  if (vma->flags_ & VMA_WRITE) rights |= MMU_WRITE;
  if (vma->flags_ & VMA_READ) rights |= MMU_READ;
  if (vma->flags_ & VMA_EXEC /* && rights */) {
    // assert ();    todo check we got the rights
    rights |= MMU_EXEC;
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
    void* src = mmu_temporary (&page);
    memcpy ((void*)address, src, PAGE_SIZE);
  }

  return __noerror();
}


#define addspace_find(s,a) kvma_look_at(s,(size_t)a)
// @todo thread_kill supposed that we are executing a task, which may not be true!
#define thread_kill(s) kpanic("PANIC, learn to kill process for SIGSEV at address [%x]\n",address);
// ---------------------------------------------------------------------------
int page_fault (void* address, int cause, bool onsystem) 
{
  kAddSpace_t* mmspc = NULL;
  if (kCPU.current_ != NULL)
    mmspc = &kCPU.current_->process_->memSpace_;

  // kprintf ("PF at %x cause %x, sys:%d \n", address, cause, onsystem);
  assert ((onsystem && (cause & PF_USER) == 0) || (!onsystem && (cause & PF_USER) != 0));
  assert ((cause & PF_PROT) == 0); // @todo handle this
  assert ((cause & PF_RSVD) == 0); // @todo handle this
  assert ((cause & PF_INSTR) == 0); // @todo handle this

  int space = mmu_addspace(address);
  if (space == AD_UNUSED) {
    if (onsystem)
      kpanic ("Kernel access invalid data at 0x%x\n", address);
    return thread_kill(SIGSEV);
  }
  if (space == AD_KERNEL) {
    if (!onsystem)
      return thread_kill(SIGSEV);
    return mmu_resolve (address, 0, MMU_KERNEL | MMU_READ | MMU_WRITE, true);
  }

  assert (space == AD_USERSP);
  assert (mmspc != NULL);

  kVma_t* vma = addspace_find(mmspc, address);
  if (vma == NULL)
    return thread_kill(SIGSEV);

  if (vma->flags_ & VMA_STACK) 
    return mmu_resolve(address, 0, MMU_READ | MMU_WRITE, true);

  if (vma->ino_ != NULL) {
    return page_inode(vma, (void*)ALIGN_DW((size_t)address, PAGE_SIZE));
    // Fill inode !
  }

  if (onsystem)
    kpanic ("Kernel access invalid data at 0x%x\n", address);
  return thread_kill(SIGSEV);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
