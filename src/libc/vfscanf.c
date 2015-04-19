#include <errno.h>
#include <stdarg.h>
#include <smkos/file.h>

int vfscanf(FILE *restrict f, const char *restrict format, va_list ap)
{
  // FIXME ...
  errno = ENOSYS;
  return -1;
}
