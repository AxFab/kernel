#include <kinfo.h>

int kseterrno(int err, const char* file, int line, const char* func)
{
  if (err) {
    kprintf("Error %d on %s:%d (%s) [%s]\n", err, file, line, func, strerror(err));
    // kstacktrace (8);
  }

  kCPU.errNo = err;
  return kCPU.errNo;
}

int kgeterrno()
{
  return kCPU.errNo;
}

/*
int kpanic (const char *str, ...)
{
  const char** args = &str;
  format ((_putc_f)kputc, 0, str, ++args);
  kTty_Update();
  for (;;);
}


void _assert (int test, const char* expression, const char* function, const char* file, int line)
{
  if (!test) {
    kprintf ("Assertion: %s at %s:%d on %s\n", expression, file, line, function);
    for (;;);
  }
}
*/

