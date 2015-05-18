#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <smkos/file.h>

/* All of those methods are bind over vfprintf
 * which is implemented in another file.
 */
int vfprintf (FILE *fp, const char *str, va_list ap);

#undef INT_MAX
#define INT_MAX 2147483648UL

#undef UINT_MAX
#define UINT_MAX 4294967296UL

// ---------------------------------------------------------------------------
/** Write on a string streaming */
static int _swrite (FILE *restrict fp, const char *restrict buf, size_t length)
{
  size_t lg = MIN (length, (size_t)(fp->wend_ - fp->wpos_));
  memcpy (fp->wpos_, buf, lg);
  fp->wpos_ += lg;
  fp->count_ += lg;
  return (length > lg) ? EOF : (int)lg;
}


// ---------------------------------------------------------------------------
/** Implementation of `vsnprintf` need to be inlined */
static inline int _vsnprintf(char *restrict str, size_t lg, const char *restrict format, va_list ap)
{
  char b;
  int res;
  FILE fp = {
    .lbuf_ = EOF,
    .write_ = _swrite,
    .lock_ = -1,
    .wpos_ = str,
    .wend_ = str + lg,
  };

  if (lg - 1 > INT_MAX - 1) {
    errno = EOVERFLOW;
    return -1;
  } else if (!lg) {
    fp.wpos_ = &b;
    fp.wend_ = fp.wpos_++;
  } else if (fp.wend_ < fp.wpos_) {
    fp.wend_ = (char *)SIZE_MAX;
  }

  res = vfprintf(&fp, format, ap);
  fp.wpos_[-(fp.wpos_ == fp.wend_)] = '\0';
  return res;
}


// ===========================================================================
#ifdef _C89
/** Write formated string on standard output */
int printf(const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vfprintf(stdout, format, ap);
  va_end(ap);
  return ret;
}


// ---------------------------------------------------------------------------
/** Write formated string on standard output */
int vprintf(const char *restrict format, va_list ap)
{
  return vfprintf(stdout, format, ap);
}


// ---------------------------------------------------------------------------
/** Write formated string on an opened file */
int fprintf(FILE *restrict fp, const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vfprintf(fp, format, ap);
  va_end(ap);
  return ret;
}


// ===========================================================================
/** Write formated string from a file descriptor */
int dprintf(int fd, const char *restrict format, ...)
{
  int ret;
  FILE fp = {
    .fd_ = fd,
    .lbuf_ = EOF,
    .write_ = _dwrite,
    .lock_ = -1
  };
  va_list ap;
  va_start(ap, format);
  ret = vfprintf(&fp, format, ap);
  va_end(ap);
  return ret;
}


// ---------------------------------------------------------------------------
/** Write formated string from a file descriptor */
int vdprintf(int fd, const char *restrict format, va_list ap)
{
  FILE fp = {
    .fd_ = fd,
    .lbuf_ = EOF,
    .write_ = _dwrite,
    .lock_ = -1
  };
  return vfprintf(&fp, format, ap);
}
#endif

// ===========================================================================
/** Write formated string from a string */
int sprintf(char *restrict str, const char *restrict format, ...)
{
  int res;
  va_list ap;
  va_start(ap, format);
  res = _vsnprintf (str, INT_MAX, format, ap);
  va_end(ap);
  return res;
}


// ---------------------------------------------------------------------------
/** Write formated string from a string */
int snprintf(char *restrict str, size_t lg, const char *restrict format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = _vsnprintf(str, lg, format, ap);
  va_end(ap);
  return ret;
}


// ---------------------------------------------------------------------------
/** Write formated string from a string */
int vsprintf(char *restrict str, const char *restrict format, va_list ap)
{
  return _vsnprintf(str, INT_MAX, format, ap);
}


// ---------------------------------------------------------------------------
/** Write formated string from a string */
int vsnprintf(char *restrict str, size_t lg, const char *restrict format, va_list ap)
{
  return _vsnprintf(str, lg, format, ap);
}

#ifdef _C89
// ===========================================================================
/** Write formated string from an allocated string */
int asprintf(char **str, const char *format, ...)
{
  int ret;
  va_list ap;
  va_start(ap, format);
  ret = vasprintf(NULL, format, ap);
  va_end(ap);
  return ret;
}


// ---------------------------------------------------------------------------
/** Write formated string from an allocated string */
int vasprintf(char **s, const char *format, va_list ap)
{
  va_list ap2;
  va_copy(ap2, ap);
  int l = vfprintf(NULL, format, ap);
  va_end(ap2);

  if (l < 0 || !(*s = malloc_(l + 1)))
    return -1;

  return _vsnprintf(*s, l + 1, format, ap);
}

#endif

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
