#include <smkos/kernel.h>
#include <smkos/klimits.h>
// #include <smkos/core.h>
// #include <smkos/arch.h>

#include "mmu.h"


/* ----------------------------------------------------------------------- */
void mmu_prolog ()
{
  memset ((void *)PG_BITMAP_ADD, 0xff, PG_BITMAP_LG);
  kSYS.pageAvailable_ = 0;
}


/* ----------------------------------------------------------------------- */
/** Function to inform paging module that some RAM can be used by the system .
  * @note this function should be used before mmu_init after mmu_prolog.
  */
void mmu_ram (int64_t base, int64_t length)
{
  uint64_t obase = base;

  if (length < 0 || base < 0 || base + length > 4 * _Gb_)
    return;

  kSYS.memMax_ = base + length;

  base = ALIGN_UP(base, PAGE_SIZE);
  length = ALIGN_DW(length - (base - obase), PAGE_SIZE);


  // The first 2 Mo is reserved to kernel
  if (base < 2 * _Mb_) {
    length -= (2 * _Mb_ - base);
    base = 2 * _Mb_;

    if (length <= 0)
      return;
  }

  if (base >= 4 * _Gb_)
    return;

  if (base + length > 4 * _Gb_)
    length = 4 * _Gb_ - base;

  base = base / PAGE_SIZE;
  length = length / PAGE_SIZE;

  kSYS.pageAvailable_ += length;
  bclearbytes ((uint8_t *)PG_BITMAP_ADD, (int)base, (int)length);
}


/* ----------------------------------------------------------------------- */
int mmu_init ()
{
  int i;
  kSYS.pageMax_ = kSYS.pageAvailable_;
  kprintf ("Memory detected %s\n", kpsize((uintmax_t)kSYS.memMax_));
  kprintf ("Memory available %s\n", kpsize(kSYS.pageAvailable_ * PAGE_SIZE));

  uint32_t *krnDir = (uint32_t *)MMU_PREALLOC_DIR;
  uint32_t *krnTbl = (uint32_t *)MMU_PREALLOC_TBL;
  memset(krnDir, 0, PAGE_SIZE);
  krnDir[0] = MMU_PREALLOC_TBL | MMU_ACCESS_WR;
  // Mirror
  krnDir[1023] = MMU_PREALLOC_DIR | MMU_ACCESS_WR;

  // The first 2 Mo is reserved to kernel
  for (i = 0; i < 512; ++i) {
    // This one is temporary
    if (i == MMU_PREALLOC_NEW / PAGE_SIZE)
      continue;

    krnTbl[i] = (i * PAGE_SIZE) | MMU_ACCESS_WR;
  }

  // @todo remove screen at 0x4
  // uint32_t *scrDir = (uint32_t *)0x4000;
  // krnDir[1] = (uint32_t)scrDir | MMU_ACCESS_WR;
  // memset (scrDir, 0, PAGE_SIZE);
  // int lg = ALIGN_UP (screen._length, PAGE_SIZE) / PAGE_SIZE;

  // if (lg > 1024) lg = 1024;

  // for (i = 0; i < lg; ++i) {
  //   scrDir[i] = (i * PAGE_SIZE + (uint32_t)screen._ptr) | MMU_ACCESS_WR;
  // }

  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
void mmu_stat()
{
  kprintf ("Memory:\n");
  kprintf ("  detected    %s\n", kpsize((uintmax_t)kSYS.memMax_));
  kprintf ("  allocatable %s\n", kpsize((uintmax_t)kSYS.pageMax_ * PAGE_SIZE));
  kprintf ("  available   %s\n", kpsize((uintmax_t)kSYS.pageAvailable_ * PAGE_SIZE));
}


/* ----------------------------------------------------------------------- */
void mmu_dump()
{
  int i, j;
  uint32_t *table = (uint32_t *)0xffc00000;
  char tmp[1040] ; //  = (char*)kalloc(1024 + 10, 0);

  for (i = 0, j = 0; i < 1024; ++i) {
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


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
