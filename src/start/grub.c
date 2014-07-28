#include "pages.h"

void VBA_Set (void* address, int width, int height, int depth);

// --------------------------------------------------------------------------
/** Read memory map */
static int kgrubMemory (uint32_t *mmap)
{
  uint64_t base;
  uint64_t length;
  // uint64_t total = 0;

  kPg_PreSystem ();
  for (; mmap[0] == 0x14; mmap += 6) {

    base = (uint64_t)mmap[1] | (uint64_t)mmap[2] << 32;
    length = (uint64_t)mmap[3] | (uint64_t)mmap[4] << 32;
    // if (base + length > total) total = base + length;
    // kprintf ("MEM RECORD [%x-%x] <%x>\n", (uint32_t)base, (uint32_t)length, (uint32_t)(total >> 32ULL));
    if (mmap[5] == 1) {
      kPg_AddRam (base, length);
		}
  }

  // kprintf ("Memory detected %d Kb\n", (uint32_t)(total / 1024) );
  kPg_Initialize ();

  return __noerror();
}

// --------------------------------------------------------------------------
/**
 * Using grub at boot, we send to the kernel all known machine infos
 */
extern char* klogStart;
extern int klogLg;

int kGrub_Initialize (uint32_t* bTable)
{
  if (bTable[0] & (1<<11) && bTable[22] != 0x000B8000) {
    kTty_PreSystem ((void*)bTable[22], bTable[25], bTable[26], 4);
    VBA_Set ((void*)bTable[22], bTable[25], bTable[26], 4);
  } else  {
    kTty_PreSystem ((void*)0x000B8000, 80, 25, 2);
  }

  memset (klogStart, 0, klogLg);
  // kprintf ("\n");
  if (bTable[0] & (1<<9)) {
    kprintf ("Boot Loader: %s\n", (char*)bTable[16]);
  }

  if (bTable[0] & (1<<1)) {
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

  if (!(bTable[0] & (1<<6))) {
      return -1;
  }

  kgrubMemory ((uint32_t*)bTable[12]);

	return __noerror();
}

