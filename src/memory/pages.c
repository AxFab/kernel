#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
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

// ---------------------------------------------------------------------------
void kpg_dump (uint32_t *table)
{
  int i;
  for (i=0; i<1024; ++i) {
    if ((i % 128) == 0)
      kTty_Putc ('\n');
    if (table[i] == 0)
      kTty_Putc ('.');
    // else
    //   kTty_Putc ((table[i] & 0x7) + '0');

    else if ((table[i] & 0x7) == 7)
      kTty_Putc ('U');
    else if ((table[i] & 0x7) == 3)
      kTty_Putc ('K');
    else if ((table[i] & 0x7) == 5)
      kTty_Putc ('R');
    else
      kTty_Putc ('?');
  }

  kTty_Putc ('\n');
}

// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context */
void kpg_resolve (uint32_t address, uint32_t *table, int rights, int dirRight, uint32_t page)
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
    memset ((void*)(address & ~(PAGE_SIZE-1)), 0, PAGE_SIZE);
  }
}


// ---------------------------------------------------------------------------
uint32_t last = 0;
int kpg_fault (uint32_t address)
{
  if (KLOG_PF) kprintf ("PF] PF at <%x> \n", address);

  if (address < USR_SPACE_BASE)
    kpanic ("PF] PG NOT ALLOWED <%x> \n", address);
  else if (address < USR_SPACE_LIMIT) {
    kAddSpace_t* mmspc = kCPU.current_->process_->memSpace_;
    kVma_t* vma = kvma_look_at (mmspc, address);
    if (vma == NULL ) {
      kpanic ("PF] Page fault in user space <%x> SIGFAULT \n", address);
      for (;;);
    } else if (vma->flags_ & VMA_STACK) {
      kpg_resolve (address, TABLE_DIR_PRC, PG_USER_RDWR, PG_USER_RDWR, 0);
      if (KLOG_PF) kprintf ("PF] Extend the stack\n");
    } else if (vma->ino_ != NULL)
      kpg_fill_stream (vma, ALIGN_DW (address, PAGE_SIZE),  PG_USER_RDWR);
    else {
      kpanic ("PF] Page fault in user space <%x> [%x] \n", address, vma);
      for (;;);
    }
  } else if (address < 0xffc00000)
    kpg_resolve (address, TABLE_DIR_KRN, PG_KERNEL_ONLY, PG_KERNEL_ONLY, 0);
  else
    kpanic ("PF] PG NOT ALLOWED <%x> \n", address);


  return __noerror();
}

// ---------------------------------------------------------------------------
uint32_t kpg_new ()
{
  int i;
  uint32_t page = kpg_alloc();
  // FIXME check this operation is possible (sti, realy clean here!)
  if (KLOG_PF) kprintf ("pg] Create a new directory <%X> \n", page);
  // kpg_dump (TABLE_DIR_THR);
  TABLE_DIR_THR [1020] = page | PG_KERNEL_ONLY;

  memset (TABLE_DIR_WIN, 0, PAGE_SIZE);
  TABLE_DIR_WIN [0] = TABLE_DIR_THR [0];
  TABLE_DIR_WIN [1] = TABLE_DIR_THR [1];  // FIXME Handle the screen better than that

  int kstart = (USR_SPACE_LIMIT >> 22) & 0x3ff;
  for (i = kstart; i < 1020; ++i)
    TABLE_DIR_WIN [i] = TABLE_DIR_THR [i];
  TABLE_DIR_WIN [1021] = TABLE_DIR_THR [1021];
  TABLE_DIR_WIN [1022] = page | PG_KERNEL_ONLY;
  TABLE_DIR_WIN [1023] = page | PG_KERNEL_ONLY;

  // kpg_dump (TABLE_DIR_WIN);
  if (KLOG_PF) kprintf ("pg] Directory is ready\n");
  // TABLE_DIR_THR [1020] = 0;

  return page;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
