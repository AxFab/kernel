#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <smkos/file.h>


/* All of those methods are bind over vfscanf 
 * which is implemented in another file.
 */
int vfscanf(FILE *restrict f, const char *restrict format, va_list ap);

#undef INT_MAX
#define INT_MAX 2147483648UL

#undef UINT_MAX
#define UINT_MAX 4294967296UL


// ---------------------------------------------------------------------------
/** Read from a string streaming */
static int _sread(FILE *restrict fp, char *restrict buf, size_t length)
{
  size_t lg = MIN (length, (size_t)(fp->rend_ - fp->rpos_));
  memcpy (buf, fp->rpos_, lg);
  fp->rpos_ += lg;
  return (length > lg) ? EOF : (int)lg;
}


// ===========================================================================
#ifdef _C89
/** Read and parse standard input */
int scanf(const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vfscanf(stdin, format, ap);
  va_end(ap);
  return ret;
}



// ---------------------------------------------------------------------------
/** Read and parse standard input */
int vscanf(const char *restrict format, va_list ap)
{
  return vfscanf(stdin, format, ap);
}


// ---------------------------------------------------------------------------
/** Read and parse an open file */
int fscanf(FILE *restrict f, const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vfscanf(f, format, ap);
  va_end(ap);
  return ret;
}

#endif

// ===========================================================================
/** Read and parse a string */
int sscanf(const char *restrict str, const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  FILE fp = {
    .rpos_ = (char *)str,
    .rend_ = (char *)SIZE_MAX,
    .read_ = _sread,
    .lock_ = -1
  };
  ret = vfscanf(&fp, format, ap);
  va_end(ap);
  return ret;
}


// ---------------------------------------------------------------------------
/** Read and parse a string */
int vsscanf(const char *restrict str, const char *restrict format, va_list ap)
{
  FILE fp = {
    .rpos_ = (char *)str,
    .rend_ = (char *)SIZE_MAX,
    .read_ = _sread,
    .lock_ = -1
  };
  return vfscanf(&fp, format, ap);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
