#include <kinfo.h>

// ----------------------------------------------------------------------------
/**
    Change the kernel error status. 
    On debug/paranoïd mode, each error are logged. 
 */
int kseterrno(int err, const char* file, int line, const char* func)
{
  if (err) {
    kprintf("Error %d on %s:%d (%s) [%s]\n", err, file, line, func, strerror(err));
    // kstacktrace (8);
  }

  kCPU.errNo = err;
  return kCPU.errNo;
}

// ----------------------------------------------------------------------------
/**
    Return last registered kernel error/failure.
 */
int kgeterrno()
{
  return kCPU.errNo;
}

// ============================================================================
#ifdef __KERNEL

// ----------------------------------------------------------------------------
/**
    The kernel enter on Panic mode, when suite of operations is compromized.
 */
int kpanic (const char *str, ...)
{
  const char** args = &str;
  format ((_putc_f)kputc, 0, str, ++args);
  kTty_Update();
  for (;;);
}


// ----------------------------------------------------------------------------
/**
    This overwrite of the assert function is made to retrieve info on kernel
 */
void _assert (int test, const char* expression, const char* function, const char* file, int line)
{
  if (!test) {
    kpanic ("Assertion: %s at %s:%d on %s\n", expression, file, line, function);
  }
}

#endif

