#include <kernel/core.h>
#include <kernel/mmu.h>
#include <kernel/scheduler.h> // TODO Task


// ---------------------------------------------------------------------------
void mmu_prolog ()
{
  memset ((void*)PG_BITMAP_ADD, 0xff, PG_BITMAP_LG);
  kSYS.pageAvailable_ = 0;
}


// ---------------------------------------------------------------------------
/** Function to inform paging module that some RAM can be used by the system .
  * @note this function should be used before mmu_init after mmu_prolog.
  */
void mmu_ram (uint64_t base, uint64_t length)
{
  uint64_t obase = base;
  if (base + length > kSYS.memMax_) 
    kSYS.memMax_ = base + length;

  base = ALIGN_UP(base, PAGE_SIZE);
  length = ALIGN_DW(length - (base - obase), PAGE_SIZE);

  base = base / PAGE_SIZE;
  length = length / PAGE_SIZE;

  if (base >= PAGE_MAX)
    return;

  // The first Mo is reserved to kernel
  if (base < 1 * _Mb_ / PAGE_SIZE) {
    if (base + length > 1 * _Mb_ / PAGE_SIZE)
      kpanic ("Unexpected boot behaviour\n");
    return;
  }

  if (base + length > PAGE_MAX)
    length = PAGE_MAX - base;

  kSYS.pageAvailable_ += length;
  bclearbytes ((uint8_t*)PG_BITMAP_ADD, (int)base, (int)length);
}


extern kTty_t screen;
// ---------------------------------------------------------------------------
int mmu_init ()
{
  int i;
  kSYS.pageMax_ = kSYS.pageAvailable_;
  kCPU.tmpPageStack_ = 1024;
  kprintf ("Memory detected %s\n", kpsize((uintmax_t)kSYS.memMax_));
  kprintf ("Memory available %s\n", kpsize(kSYS.pageAvailable_ * PAGE_SIZE));

  uint32_t* krnDir = (uint32_t*)MMU_PREALLOC_DIR;
  uint32_t* krnTbl = (uint32_t*)MMU_PREALLOC_TBL;
  memset(krnDir, 0, PAGE_SIZE);
  krnDir[0] = MMU_PREALLOC_TBL | MMU_ACCESS_WR;
    // Mirror
  krnDir[1023] = MMU_PREALLOC_DIR | MMU_ACCESS_WR;

  // The first Mo is reserved to kernel
  for (i = 0; i< 256; ++i) {
    // This one is temporary
    if (i == MMU_PREALLOC_NEW / PAGE_SIZE) 
      continue;

    krnTbl[i] = (i * PAGE_SIZE) | MMU_ACCESS_WR;
  }

  // @todo remove screen at 0x4
  uint32_t* scrDir = (uint32_t*)0x4000;
  krnDir[1] = (uint32_t)scrDir | MMU_ACCESS_WR;
  memset (scrDir, 0, PAGE_SIZE);
  int lg = ALIGN_UP (screen._length, PAGE_SIZE) / PAGE_SIZE;
  if (lg > 1024) lg = 1024;
  for (i = 0; i< lg; ++i) {
    scrDir[i] = (i * PAGE_SIZE + (uint32_t)screen._ptr) | MMU_ACCESS_WR;
  }

  return __seterrno(0);
}


// ---------------------------------------------------------------------------
/** Allocat a single page for the system and return it's physical address */
page_t mmu_newpage()
{
  static uint8_t* bitmap = (uint8_t*)PG_BITMAP_ADD;
  int i=0, j=0;
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


// ---------------------------------------------------------------------------
/** Mark a single physique page, returned by mmu_newpage, as available again */
void mmu_release(page_t page)
{
  static uint8_t* bitmap = (uint8_t*)PG_BITMAP_ADD;
  int i = (page / PAGE_SIZE) / 8;
  int j = (page / PAGE_SIZE) % 8;

  if (i >= PG_BITMAP_LG || (bitmap[i] & (1 << j)) == 0)
    kpanic ("Release page with wrong args\n");

  bitmap[i] = bitmap[i] & (~(1 << j));
}


// ---------------------------------------------------------------------------
void mmu_stat()
{
  kprintf ("Memory:\n");
  kprintf ("  detected    %s\n", kpsize((uintmax_t)kSYS.memMax_));
  kprintf ("  allocatable %s\n", kpsize((uintmax_t)kSYS.pageMax_ * PAGE_SIZE));
  kprintf ("  available   %s\n", kpsize((uintmax_t)kSYS.pageAvailable_ * PAGE_SIZE));
}

// ---------------------------------------------------------------------------
void mmu_dump()
{
  int i, j;
  uint32_t* table = (uint32_t*)0xffc00000;
  char tmp[1040] ; //  = (char*)kalloc(1024 + 10, 0);

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
  kprintf(tmp);
}
// ---------------------------------------------------------------------------
int mmu_resolve (size_t address, page_t page, int access, bool zero)
{
  assert (zero || page != 0);
  assert (!zero || page == 0);

  int dir = (address >> 22) & 0x3ff;
  int tbl = (address >> 12) & 0x3ff;

  // kprintf ("resolve page fault at _[%d][%d] for %x - %x\n", dir, tbl, address, add);
  uint32_t* table = (access & VMA_KERNEL) ? MMU_LEVEL1_KRN() : MMU_LEVEL1();
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
      memset((void*)(address & ~(PAGE_SIZE-1)), 0, PAGE_SIZE);
  }

  return __seterrno(0);
}



// ---------------------------------------------------------------------------
page_t mmu_new_dir() 
{
  int i;
  static uint32_t* const kernelDir = (uint32_t*)MMU_PREALLOC_DIR;

  klock(&kCPU.current_->process_->lock_);
  if (kCPU.current_->process_->pageDir_ == 0) {

    // Map a page to a temporary address
    page_t page = mmu_newpage();
    ++kCPU.current_->process_->privatePage_;
    uint32_t* table = (uint32_t*)MMU_PREALLOC_TBL;
    table[MMU_PREALLOC_NEW / PAGE_SIZE] = page | MMU_ACCESS_WR;
    uint32_t* dir = (uint32_t*)MMU_PREALLOC_NEW;

    memset(dir, 0, PAGE_SIZE);

    // Kernel first Mb
    dir[0] = MMU_PREALLOC_TBL | MMU_ACCESS_WR;
    dir[1] = 0x4000 | MMU_ACCESS_WR; // @todo hack for early screen

    // Mirror
    dir[1023] = page | MMU_ACCESS_WR;

    // Copy kernel heap -- not mandatory but avoid useless soft-pagefaults.
    int kstart = (MMU_USERSP_LIMIT >> 22) & 0x3ff;
    for (i = kstart; i < 1022; ++i)
      dir[i] = kernelDir[i];

    // No need to inval TBL, we swap dir right now
    table[MMU_PREALLOC_NEW / PAGE_SIZE] = 0; 
    kCPU.current_->process_->pageDir_ = page;
  }
  kunlock(&kCPU.current_->process_->lock_);
  return kCPU.current_->process_->pageDir_;
}


// ---------------------------------------------------------------------------
// @todo remove this one, should use kmap/kunmap mechanisms (kBucket_t).
void* mmu_temporary (page_t* page)
{
  if (MMU_LEVEL1()[1020] == 0) 
    MMU_LEVEL1()[1020] = mmu_newpage() | MMU_ACCESS_WR;

  if (*page == 0)
    *page = mmu_newpage();
  MMU_LEVEL2(1020)[--kCPU.tmpPageStack_] = *page | MMU_ACCESS_WR;
  return (void*)(0xff000000 + kCPU.tmpPageStack_ * 0x1000);
}


// ---------------------------------------------------------------------------
void mmu_reset_stack ()
{
  kCPU.tmpPageStack_ = 1024;
  if (MMU_LEVEL1()[1020] != 0) {
    int i;
    for (i=0; i<1024; ++i)
      MMU_LEVEL2(1020)[i] = 0;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
