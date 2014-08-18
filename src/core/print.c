#include <kernel/core.h>

#ifdef __KERNEL

#include <ax/file.h>
void kTty_Update();

char* klogStart = (char*)0x7000;
int klogLg = (0x10000 - 0x7000);
int kLogPen = 0;

int _kwrite (FILE* fp, const char* buf, size_t length)
{
  int lg = length;
  while (lg--) {
    int c = *(buf++);
    if (c <= 0 || c >= 0x80) {
      c = 0x7f;
    }

    klogStart[kLogPen++] = (c & 0x7f);

    if (kLogPen >= klogLg) {
      kLogPen = 0;
    }
  }

  fp->count_ += length;
  return length;
}


#undef kprintf


int kvprintf (const char* str, va_list ap)
{
  FILE fp = {
    .count_ = 0,
    .lbuf_ = 0,
    .write_ = _kwrite,
    .lock_ = -1,
  };

  int res = vfprintf (&fp, str, ap);
  kTty_Update(); // FIXME this is only for kernel raw debugging
  return res;
}

int kprintf (const char* str, ...)
{
  va_list ap;
  va_start(ap, str);
  int v = kvprintf (str, ap);
  va_end(ap);
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

