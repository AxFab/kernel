#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/kstruct/user.h>
#include <stdarg.h>

void kwrite(void* buf, int lg);
// int vsnprintf(char* buf, int lg, const char* fmt, va_list ap);

/* ----------------------------------------------------------------------- */
void kpanic(const char *msg, ...)
{
  int lg;
  char buf[256];
  va_list ap;
  va_start(ap, msg);
  lg = vsnprintf(buf, 256, msg, ap);
  va_end(ap);
  kwrite(buf, lg);
  kstacktrace(12);
  cpu_halt();
}

/* ----------------------------------------------------------------------- */
struct spinlock sysLogLock;
void kprintf(const char *msg, ...)
{
  int lg;
  char buf[256];
  va_list ap;
  klock(&sysLogLock);
  va_start(ap, msg);
  lg = vsnprintf(buf, 256, msg, ap);
  va_end(ap);
  kwrite(buf, lg);
  kunlock(&sysLogLock);
}


/* ----------------------------------------------------------------------- */
/** Store in a temporary buffer a size in bytes in a human-friendly format. */
const char *kpsize (uintmax_t number)
{
  static char sz_format[20];
  static const char *prefix[] = { "bs", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb" };
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


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
