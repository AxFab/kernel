#include <kernel/core.h>
#include <stdarg.h>


// ----------------------------------------------------------------------------
/**
    Return last registered kernel error/failure.
 */
int *_geterrno()
{
  return kCPU.errNo;
}

// ============================================================================
#ifdef __KERNEL

void kTty_Update(); // TODO Fix header mess

// ----------------------------------------------------------------------------
/**
    The kernel enter on Panic mode, when suite of operations is compromized.
 */

int kpanic (const char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  kvprintf (str, ap);
  va_end(ap);

  kstacktrace (10);
  kTty_Update();

  for (;;);
}

void throw()
{
  kpanic ("Kernel throw: Stack overflow\n");
}

// ----------------------------------------------------------------------------
/**
    This overwrite of the assert function is made to retrieve info on kernel
 */
void _assert (int test, const char *expression, const char *function, const char *file, int line)
{
  if (!test) {
    kpanic ("Assertion: %s at %s:%d on %s\n", expression, file, line, function);
  }
}

#endif

