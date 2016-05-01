#include <smkos/kapi.h>


/* Special entry point for VGA initalizaation during boot */
void VGA_Info(size_t add, int width, int height, int depth);

/* ----------------------------------------------------------------------- */
/** Read memory map */
static int grub_memory (uint32_t *mmap)
{
  int64_t base;
  int64_t length;

  mmu_prolog();

  for (; mmap[0] == 0x14; mmap += 6) {

    base = (int64_t)mmap[1] | (int64_t)mmap[2] << 32;
    length = (int64_t)mmap[3] | (int64_t)mmap[4] << 32;

    if (mmap[5] == 1)
      mmu_ram(base, length);
  }

  mmu_init();
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** Using grub at boot, we send to the kernel all known machine infos. */
int grub_initialize (uint32_t *bTable)
{
  if (bTable[0] & (1 << 11) && bTable[22] != 0x000B8000)
    VGA_Info(bTable[22], bTable[25], bTable[26], 4);
  else
    memset((void *)0x000B8000, 0, 400);

  if (bTable[0] & (1 << 9))
    kprintf ("Boot Loader: %s\n", (char *)bTable[16]);

  if (bTable[0] & (1 << 2))
    kprintf ("Command Line: %s\n", (char *)bTable[4]);

  if (bTable[0] & (1 << 1)) {
    kprintf ("Booting device: ");

    if (bTable[3] >> 24 == 0x80)
      kprintf ("HDD\n");
    else if (bTable[3] >> 24 == 0x80)
      kprintf ("HDD\n");
    else if (bTable[3] >> 24 == 0xe0)
      kprintf ("CD\n");
    else
      kprintf ("Unknown <%x>\n", bTable[3] >> 24);
  }

  if (!(bTable[0] & (1 << 6))) {
    return -1;
  }

  grub_memory ((uint32_t *)bTable[12]);
  return __seterrno(0);
}


int *__errno_location()
{
  return &kCPU.errno_;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
