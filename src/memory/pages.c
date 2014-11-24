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
#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/vfs.h>
#include <kernel/info.h>

/**
  Table   xxxx----HDA--UWP b

  P: Is the page present in memory
  W: Is the page writable
  U: Is the page accessible in user space
  A: Read operation happends
  D: Write operation happends
  H: Huge page

 */

#ifdef __KERNEL
// ---------------------------------------------------------------------------
void kpg_dump (uint32_t *table)
{
  int i, j;
  char* tmp = (char*)kalloc (1024 + 10);

  for (i=0, j=0; i<1024; ++i) {
    if ((i % 128) == 0)
      tmp[j++] = '\n';
    if (table[i] == 0)
      tmp[j++] = '.';
    else if ((table[i] & 0x7) == 7)
      tmp[j++] = 'u';
    else if ((table[i] & 0x7) == 3)
      tmp[j++] = 'k';
    else if ((table[i] & 0x7) == 5)
      tmp[j++] = 'r';
    else
      tmp[j++] = '?';
  }

  tmp[j++] = '\n';
  tmp[j++] = '\0';
  kprintf (tmp);
}


// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context */
void kpg_resolve (uint32_t address, uint32_t *table, int rights, int dirRight, uint32_t page, int reset)
{
  int dir = (address >> 22) & 0x3ff;
  int tbl = (address >> 12) & 0x3ff;

  if (KLOG_PF) kprintf ("PF] Resolve at %x with %d,%d <%d,%d>\n", address, rights, dirRight, dir, tbl);
  if (table[dir] == 0) {
    uint32_t page = kpg_alloc();
    table[dir] = page | dirRight;

    if (table != TABLE_DIR_THR) {
      TABLE_DIR_THR[dir] = table[dir] | dirRight;
    }

    memset (TABLE_PAGE_TH(dir), 0, PAGE_SIZE);
  }

  if (TABLE_PAGE_TH(dir)[tbl] == 0) {
    if (page == 0)
      page = kpg_alloc();
    TABLE_PAGE_TH(dir)[tbl] = page | rights;
    if (reset)
      memset ((void*)(address & ~(PAGE_SIZE-1)), 0, PAGE_SIZE);
  }
}

// ---------------------------------------------------------------------------
int kpg_resolve_inode (kVma_t* vma, uint32_t address, int rights)
{
  uint32_t page;

  size_t off = (vma->offset_ + (address - vma->base_));
  if (KLOG_PF) kprintf ("PF] stream at <%x>  [%x-%x-%x]\n", address, vma->offset_, vma->base_, off);

  if (rights == PG_USER_RDWR)
    rights = vma->flags_ & VMA_WRITE ? PG_USER_RDWR : PG_USER_RDONLY;

  if (inode_page (vma->ino_, off, &page))
    return __geterrno ();

  if (vma->flags_ & VMA_SHARED) {
    kpg_resolve (address, TABLE_DIR_PRC, rights, PG_USER_RDWR, page, FALSE);

  } else {
    // FIXME Should use copy-on-write (Resolve / PG_USER_RDONLY -> copy after (inval))
    uint32_t copy = kpg_alloc();
    kpg_resolve (address, TABLE_DIR_PRC, rights, PG_USER_RDWR, copy, FALSE);
    void* src = kpg_temp_page (&page);
    memcpy ((void*)address, src, PAGE_SIZE);
  }

  if (KLOG_PF) kprintf ("PF] fill stream at <%x> \n", address);
  return __noerror();
}

// ---------------------------------------------------------------------------
uint32_t last = 0;
int kpg_fault (uint32_t address)
{
  if (KLOG_PF) kprintf ("PF] PF at <%x> \n", address);
  kAddSpace_t* mmspc = NULL;
  if (kCPU.current_ != NULL)
    mmspc = kCPU.current_->process_->memSpace_;


  if (address < kHDW.userSpaceBase_)
    kpanic ("PF] PG NOT ALLOWED <%x-%d-%d> \n", address, (address >> 22) & 0x3ff, (address >> 12) & 0x3ff);
  else if (address < kHDW.userSpaceLimit_) {
    kVma_t* vma = kvma_look_at (mmspc, address);
    if (vma == NULL ) {
      kpanic ("PF] Page fault in user space <%x> SIGFAULT \n", address);
      for (;;);
    } else if (vma->flags_ & VMA_STACK) {
      kpg_resolve (address, TABLE_DIR_PRC, PG_USER_RDWR, PG_USER_RDWR, 0, TRUE);
      if (KLOG_PF) kprintf ("PF] Extend the stack\n");
    } else if (vma->ino_ != NULL)
      kpg_resolve_inode (vma, ALIGN_DW (address, PAGE_SIZE), PG_USER_RDWR);
      // kpanic ("PF] Page fault on mapped file !? <%x> [%x] %s \n", address, vma, vma->ino_->name_);
    else {
      kpanic ("PF] Page fault in user space <%x> [%x] \n", address, vma);
      for (;;);
    }
  } else if (address < 0xff000000)
    kpg_resolve (address, TABLE_DIR_KRN, PG_KERNEL_ONLY, PG_KERNEL_ONLY, 0, TRUE);
  else
    kpanic ("PF] PG NOT ALLOWED <%x-%d-%d> [%d]\n", address, (address >> 22) & 0x3ff, (address >> 12) & 0x3ff, kCPU.tmpPageStack_);

  if (KLOG_PF) kprintf ("PF] PF end\n");
  return __noerror();
}


// ---------------------------------------------------------------------------
void* kpg_temp_page (uint32_t* pg)
{
  // FIXME ASSERT WE ARE INSIDE A LOCK !
  // FIXME ASSERT kpg_page_stack > 0
  if (TABLE_DIR_THR [1020] == 0)
    TABLE_DIR_THR [1020] = kpg_alloc() | PG_KERNEL_ONLY;

  if (*pg == 0)
    *pg = kpg_alloc();
  TABLE_PAGE_TH(1020)[--kCPU.tmpPageStack_] = *pg | PG_KERNEL_ONLY;

  // printf("TMP PAGE at %d\n", kCPU.tmpPageStack_);
  // kstacktrace(8);
  return (void*)(0xff000000 + kCPU.tmpPageStack_ * 0x1000);
}


// ---------------------------------------------------------------------------
void kpg_reset_stack ()
{
  int i;
  if (TABLE_DIR_THR [1020] != 0) {
    for (i=0; i<1024; ++i)
      TABLE_PAGE_TH(1020)[i] = 0;
  }

  kCPU.tmpPageStack_ = 1024;
}


// ---------------------------------------------------------------------------
uint32_t kpg_new ()
{
  int i;
  uint32_t page = 0;
  uint32_t* crD = kpg_temp_page (&page);

  // FIXME check this operation is possible (sti, realy clean here!)
  if (KLOG_PF) kprintf ("pg] Create a new directory <%X> \n", page);
  // kpg_dump (TABLE_DIR_THR);


  memset (crD, 0, PAGE_SIZE);
  crD [0] = TABLE_DIR_THR [0];
  crD [1] = TABLE_DIR_THR [1];  // FIXME Handle the screen better than that

  int kstart = (kHDW.userSpaceLimit_ >> 22) & 0x3ff;
  for (i = kstart; i < 1020; ++i)
    crD [i] = TABLE_DIR_THR [i];
  crD [1021] = TABLE_DIR_THR [1021];
  crD [1022] = page | PG_KERNEL_ONLY;
  crD [1023] = page | PG_KERNEL_ONLY;

  // kpg_dump (TABLE_DIR_WIN);
  if (KLOG_PF) kprintf ("pg] Directory is ready\n");
  // TABLE_DIR_THR [1020] = 0;

  return page;
}

#endif
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
