#include <kernel/core.h>

#ifdef __KERNEL

#include <format.h>
void kTty_Update();

char* klogStart = (char*)0x7000;
int klogLg = (0x10000 - 0x7000);
int kLogPen = 0;

int kputc (int c)
{
  if (c <= 0 || c >= 0x80) {
    c = 0x7f;
  }

  klogStart[kLogPen++] = (c & 0x7f);

  if (kLogPen >= klogLg) {
    kLogPen = 0;
    kpanic ("END OF STREAM KLOG\n");
  }

  return c;
}


#undef kprintf

int kprintf (const char* str, ...)
{
  const char** args = &str;
  int v =  format ((_putc_f)kputc, 0, str, ++args);
  kTty_Update(); // FIXME this is only for kernel raw debugging
  return v;
}

#endif /* __KERNEL */


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

