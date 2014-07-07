#include <kcore.h>

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

  if (kLogPen >= klogLg)
    kLogPen = 0;

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

