#include <kcore.h>

// ----------------------------------------------------------------------------
/**
    Print the stack trace of the current frame
    FIXME: We can improve readablility by parsing the .map file
 */
void kstacktrace(uintptr_t MaxFrames)
{
  uintptr_t frame;
  uintptr_t* ebp = &MaxFrames - 2;
  kprintf("Stack trace: [%x]\n", (uintptr_t)ebp);

  for (frame = 0; frame < MaxFrames; ++frame) {
    uintptr_t eip = ebp[1];

    if (eip == 0)
      // No caller on stack
      break;

    // Unwind to previous stack frame
    ebp = (uintptr_t*)(ebp[0]);
    uintptr_t* arguments = &ebp[2];
    kprintf("  0x%x    [%x - %x] \n", eip, (uintptr_t)ebp, (uintptr_t)arguments);
  }
}

// ----------------------------------------------------------------------------
/**
    Display hexadeciaml data of a memory area.
 */
void kdump (void* ptr, size_t lg)
{
  int i;

  while (lg > 0) {
    kprintf ("0x%8x  ", (unsigned int)ptr);

    for (i = 0; i < 16; ++i)
      kprintf (" %02x", ((uint8_t*)ptr)[i]);

    kprintf ("  ");

    for (i = 0; i < 16; ++i) {
      if (((uint8_t*)ptr)[i] < 0x20) kprintf (".");

      else if (((uint8_t*)ptr)[i] > 0x7f) kprintf (".");

      else kprintf ("%c", ((uint8_t*)ptr)[i]);
    }

    kprintf ("\n");
    lg -= 16;
    ptr = &((uint8_t*)ptr)[16];
  }

  kprintf ("\n");
}

// ----------------------------------------------------------------------------
static char sz_format[20];
/**
    Store in a temporary buffer a size in bytes in a human-friendly format.
 */
const char* kpsize (uintmax_t number)
{
  const char* prefix[] = { "bs", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb" };
  int k = 0;
  int rest = 0;

  while (number > 1024) {
    k++;
    rest = number & (1024 - 1);
    number /= 1024;
  };

  if (k == 0) {
    snprintf (sz_format, 20, "%d bytes", (int)number);

  } else if (number < 10) {
    float value = (rest / 1024.0f) * 100;
    snprintf (sz_format, 20, "%1d.%02d %s", (int)number, (int)value, prefix[k]);

  } else if (number < 100) {
    float value = (rest / 1024.0f) * 10;
    snprintf (sz_format, 20, "%2d.%01d %s", (int)number, (int)value, prefix[k]);

  } else {
    // float value = (rest / 1024.0f) + number;
    //snprintf (sz_format, 20, "%.3f %s", (float)value, prefix[k]);
    snprintf (sz_format, 20, "%4d %s", (int)number, prefix[k]);
  }

  return sz_format;
}

