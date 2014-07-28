/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Bitmap page allocation
 */
#include <kernel/memory.h>


#define MMU_KERNEL                (3)
#define MMU_USER                  (7)
#define PHYS(p,r)         ((uint32_t)(p) | (r))
static uint32_t pageAvailable = 0;
static uint64_t memMax = 0;
int kpg_step = 0;
extern kTty_t screen;


// ===========================================================================
/** Initialize kernel pagination using available memory */
void kpg_init (void)
{
  int i;
  uint32_t* fpage =         (uint32_t*) 0x3000;
  uint32_t* kernelPage =    (uint32_t*) 0x2000;
  uint32_t* pageScreen =    (uint32_t*) 0x4000;

  kprintf ("Memory detected %s\n", kpsize((uintmax_t)memMax));
  kprintf ("Memory available %s\n", kpsize(pageAvailable * PAGE_SIZE));

  memset ((void*)kernelPage, 0, PAGE_SIZE);
  kernelPage[0] = PHYS (fpage, MMU_KERNEL);
  kernelPage[1021] = PHYS (kernelPage, MMU_KERNEL);
  kernelPage[1022] = PHYS (kernelPage, MMU_KERNEL);
  kernelPage[1023] = PHYS (kernelPage, MMU_KERNEL);
  for (i = 0; i< 1024/4; ++i) {
    if (i != 5)
      fpage[i] = PHYS (i * PAGE_SIZE, MMU_KERNEL);
  }


  // SCREEN
  if (screen._mode == 0)
    return;

  // TODO MAP USING REGULAR WAY !!!
  kernelPage[1] = PHYS (pageScreen, MMU_KERNEL);
  memset ((void*)pageScreen, 0, PAGE_SIZE);
  if ((uint32_t)screen._ptr & (PAGE_SIZE-1)) {
    kpanic ("BIG FAILED, screen is not align\n");
    for (;;);
  }

  int lg = ALIGN_UP (screen._length, PAGE_SIZE) / PAGE_SIZE;
  if (lg > 1024) lg = 1024;
  for (i = 0; i< lg; ++i) {
    pageScreen [i] = PHYS (i * PAGE_SIZE + (uint32_t)screen._ptr, MMU_KERNEL);
  }
}


// ---------------------------------------------------------------------------
/** Allocat a 4k page for the system and return it's physical address */
uintptr_t kpg_alloc (void)
{
  uint8_t* bitmap = (uint8_t*)PG_BITMAP_ADD;
  int i=0, j=0;
  while (bitmap[i] == 0xFF && i < PG_BITMAP_LG)
    i++;

  if (i >= PG_BITMAP_LG) {
    kpanic ("Not a single page available\n");
  }
  uint8_t value = bitmap[i];
  while (value & 1) {
    ++j;
    value = value >> 1;
  }

  bitmap[i] = bitmap[i] | (1 << j);
  pageAvailable--;

  return (i * 8 + j) * PAGE_SIZE;
}


// ---------------------------------------------------------------------------
/** Mark a physique 4k page, returned by kpg_alloc, as available */
void kpg_release (uintptr_t page)
{
  uint8_t* bitmap = (uint8_t*)PG_BITMAP_ADD;
  int i = (page / PAGE_SIZE) / 8;
  int j = (page / PAGE_SIZE) % 8;

  if (i >= PG_BITMAP_LG || (bitmap[i] & (1 << j)) == 0) {
    kpanic ("Release page with wrong args\n");
  }

  bitmap[i] = bitmap[i] & (~(1 << j));
}


// ---------------------------------------------------------------------------
/** Function to inform paging module that some RAM can be used by the system .
 * \note this function should be used before kpg_init.
 */
void kpg_ram (uint64_t base, uint64_t length)
{
  if (kpg_step == 0) {
    memset ((void*)PG_BITMAP_ADD, 0xff, PG_BITMAP_LG);
    pageAvailable = 0;
    kpg_step++;
  } else if (kpg_step > 1)
    return;

  unsigned int obase = base;
  if (base + length > memMax) memMax = base + length;

  base = (base + PAGE_SIZE - 1ULL) & ~(PAGE_SIZE - 1ULL);
  length = (length - (base - obase)) & ~(PAGE_SIZE - 1ULL);

  base = base / PAGE_SIZE;
  length = length / PAGE_SIZE;

  if (base >= 4ULL * _Gb_ / PAGE_SIZE)
    return;
  if (base < 1 * _Mb_ / PAGE_SIZE) {// The first Mo is reserved to kernel
    if (base + length > 1 * _Mb_ / PAGE_SIZE)
      kpanic ("Unexpected boot behaviour\n");
    return;
  }

  if (base + length > 4ULL * _Gb_ / PAGE_SIZE)
    length = 4ULL * _Gb_ - base;

  pageAvailable += length;
  bclearbytes ((uint8_t*)PG_BITMAP_ADD, base, length);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
