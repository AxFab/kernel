#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <smkos/file.h>
// #include <ax/slib.h>


extern const char *_utoa_digits;
extern const char *_utoa_digitsX;

#define ALT_FORM   (1<<('#'-' '))
#define ZERO_PAD   (1<<('0'-' '))
#define LEFT_ADJ   (1<<('-'-' '))
#define PAD_POS    (1<<(' '-' '))
#define MARK_POS   (1<<('+'-' '))
#define GROUPED    (1<<('\''-' '))

#define FLAGMASK (ALT_FORM|ZERO_PAD|LEFT_ADJ|PAD_POS|MARK_POS|GROUPED)


#if UINT_MAX == ULONG_MAX
#define LONG_IS_INT
#endif

#if SIZE_MAX != ULONG_MAX || UINTMAX_MAX != ULLONG_MAX
#define ODD_TYPES
#endif

enum {
  BARE, LPRE, LLPRE, HPRE, HHPRE, BIGLPRE,
  ZTPRE, JPRE,
  STOP,
  PTR, INT, UINT, ULLONG,
#ifndef LONG_IS_INT
  LONG, ULONG,
#else
#define LONG INT
#define ULONG UINT
#endif
  SHORT, USHORT, CHAR, UCHAR,
#ifdef ODD_TYPES
  LLONG, SIZET, IMAX, UMAX, PDIFF, UIPTR,
#else
#define LLONG ULLONG
#define SIZET ULONG
#define IMAX LLONG
#define UMAX ULLONG
#define PDIFF LONG
#define UIPTR ULONG
#endif
  DBL, LDBL,
  NOARG,
  MAXSTATE
};


static inline void pop_arg(union SMK_FmtArg *arg, int type, va_list *ap)
{
  /* Give the compiler a hint for optimizing the switch. */
  if ((unsigned)type > MAXSTATE) return;

  switch (type) {
  case PTR:
    arg->p = va_arg(*ap, void *);
    break;

  case INT:
    arg->s = va_arg(*ap, int);
    break;

  case UINT:
    arg->i = va_arg(*ap, unsigned int);
    break;
#ifndef LONG_IS_INT

  case LONG:
    arg->s = va_arg(*ap, long);
    break;

  case ULONG:
    arg->i = va_arg(*ap, unsigned long);
    break;
#endif

  case ULLONG:
    arg->i = va_arg(*ap, unsigned long long);
    break;

  case SHORT:
    arg->s = (short)va_arg(*ap, int);
    break;

  case USHORT:
    arg->i = (unsigned short)va_arg(*ap, int);
    break;

  case CHAR:
    arg->s = (signed char)va_arg(*ap, int);
    break;

  case UCHAR:
    arg->i = (unsigned char)va_arg(*ap, int);
    break;
#ifdef ODD_TYPES

  case LLONG:
    arg->s = va_arg(*ap, long long);
    break;

  case SIZET:
    arg->i = va_arg(*ap, size_t);
    break;

  case IMAX:
    arg->s = va_arg(*ap, intmax_t);
    break;

  case UMAX:
    arg->i = va_arg(*ap, uintmax_t);
    break;

  case PDIFF:
    arg->i = va_arg(*ap, ptrdiff_t);
    break;

  case UIPTR:
    arg->i = (uintptr_t)va_arg(*ap, void *);
    break;
#endif

  case DBL:
    arg->f = va_arg(*ap, double);
    break;

  case LDBL:
    arg->f = va_arg(*ap, long double);
    break;
  }
}

#define _I(x) [(x)-'A']

static const char state[]_I('z' + 1) = {
  {
    // BARE
    _I('d') = INT,   _I('i') = INT,
    _I('o') = UINT,  _I('u') = UINT,  _I('x') = UINT,  _I('X') = UINT,
    _I('e') = DBL,   _I('E') = DBL,   _I('f') = DBL,   _I('F') = DBL,
    _I('g') = DBL,   _I('G') = DBL,   _I('a') = DBL,   _I('A') = DBL,
    _I('c') = CHAR,  _I('C') = INT,
    _I('s') = PTR,   _I('S') = PTR,   _I('p') = UIPTR, _I('n') = PTR,
    _I('m') = NOARG, _I('l') = LPRE,  _I('h') = HPRE,  _I('L') = BIGLPRE,
    _I('z') = ZTPRE, _I('j') = JPRE,  _I('t') = ZTPRE,
  },
  {
    // LPRE
    _I('d') = LONG,  _I('i') = LONG,
    _I('o') = ULONG, _I('u') = ULONG, _I('x') = ULONG, _I('X') = ULONG,
    _I('e') = DBL,   _I('E') = DBL,   _I('f') = DBL,   _I('F') = DBL,
    _I('g') = DBL,   _I('G') = DBL,   _I('a') = DBL,   _I('A') = DBL,
    _I('c') = CHAR,  _I('s') = PTR,   _I('n') = PTR,   _I('l') = LLPRE,
  },
  {
    // LLPRE
    _I('d') = LLONG, _I('i') = LLONG,
    _I('o') = ULLONG, _I('u') = ULLONG, _I('x') = ULLONG, _I('X') = ULLONG,
    _I('n') = PTR,
  },
  {
    // HPRE
    _I('d') = SHORT, _I('i') = SHORT,
    _I('o') = USHORT, _I('u') = USHORT, _I('x') = USHORT, _I('X') = USHORT,
    _I('n') = PTR,   _I('h') = HHPRE,
  },
  {
    // HHPRE
    _I('d') = CHAR,  _I('i') = CHAR,
    _I('o') = UCHAR, _I('u') = UCHAR, _I('x') = UCHAR, _I('X') = UCHAR,
    _I('n') = PTR,
  },
  {
    // BIGLPRE
    _I('e') = LDBL,  _I('E') = LDBL,  _I('f') = LDBL,  _I('F') = LDBL,
    _I('g') = LDBL,  _I('G') = LDBL,  _I('a') = LDBL,  _I('A') = LDBL,
    _I('n') = PTR,
  },
  {
    // ZTPRE
    _I('d') = PDIFF, _I('i') = PDIFF,
    _I('o') = SIZET, _I('u') = SIZET, _I('x') = SIZET, _I('X') = SIZET,
    _I('n') = PTR,
  },
  {
    // JPRE
    _I('d') = IMAX,  _I('i') = IMAX,
    _I('o') = UMAX,  _I('u') = UMAX,  _I('x') = UMAX,  _I('X') = UMAX,
    _I('n') = PTR,
  },
};

static inline int read_format_specifier (struct SMK_FmtSpec *sb, const char **ptr, va_list *ap)
{
  char sign;
  const char *str = *ptr;

  if (isdigit(str[0]) && str[1] == '$') {
    // FIXME change arg position
  }

  // Read modifier flags
  for (sb->flag_ = 0; (*str <= '0') && ((1 << (*str - ' ')) & FLAGMASK); str++)
    sb->flag_ |= ((1 << (*str - ' ')) & FLAGMASK);


  // Read field width
  if (*str == '*') {
    str++;
    sb->field_ = va_arg(*ap, int);
  } else {
    sb->field_ = (int)_strtox(str, (char **)&str, 10, &sign);
  }

  // Read precision
  if (str[0] == '.' && str[1] == '*') {
    str += 2;
    sb->precis_ = va_arg(*ap, int);
  } else if (str[0] == '.') {
    str++;
    sb->precis_ = (int)_strtox(str, (char **)&str, 10, &sign);
  } else {
    sb->precis_ = -1;
  }

  // Format specifier state machine
  int ty = BARE;

  do {
    if (*str > 'z') return -1;

    sb->type_ = ty;
    ty = state[ty]_I(*(str++));
  } while (ty - 1 < STOP);

  sb->type_ = ty;

  if (!ty)
    return -1;

  // Check validity of argument type

  *ptr = str;
  return 0;
}

int vfprintf (FILE *fp, const char *str, va_list ap)
{
  int lg;
  char ch;
  union SMK_FmtArg arg;
  char tmp [512];
  struct SMK_FmtSpec sb;

  FLOCK(fp);

  while (*str) {

    // Write litteral characters
    if (*str != '%') {
      if (fp->write_(fp, str++, 1) < 0)
        return -1;

      continue;

      // Handle %% escape code
    } else if (str[1] == '%') {
      if (fp->write_(fp, str, 1) < 0)
        return -1;

      str += 2;
      continue;
    }

    // Read format specifier
    str++;

    if (read_format_specifier(&sb, &str, &ap) < 0)
      return -1;


    if (sb.flag_ & LEFT_ADJ)
      sb.flag_ &= ~ZERO_PAD;

    pop_arg (&arg, sb.type_, &ap);
    lg = 0;

    switch (str[-1]) {
    case 'n':
      break;

    case 'p':
      sb.flag_ = ALT_FORM | ZERO_PAD;

    case 'x':
    case 'X':

      if (sb.flag_ & ALT_FORM)
        lg -= 2;

      if (str[-1] & 32)
        _utoa(arg.i, tmp, 16, _utoa_digits);
      else
        _utoa(arg.i, tmp, 16, _utoa_digitsX);

      break;

    case 'o':
      _utoa(arg.i, tmp, 8, _utoa_digits);
      break;

    case 'u':
      _utoa(arg.i, tmp, 10, _utoa_digits);
      break;

    case 'd':
    case 'i':

      if (arg.s >= 0)
        _utoa(arg.s, tmp, 10, _utoa_digits);
      else {
        lg--;

        if (fp->write_(fp, "-", 1) < 0)
          return -1;

        _utoa(-arg.s, tmp, 10, _utoa_digits);
      }

      break;

    case 'c':
      ch = arg.s;

      if (fp->write_(fp, &ch, 1) < 0)
        return -1;

      break;

    case 's':

      if (arg.p == NULL)
        arg.p = "(null)";

      lg = strlen((char *)arg.p);

      if (fp->write_(fp, (char *)arg.p, lg) < 0)
        return -1;

      break;
    }


    switch (str[-1]) {

    case 'p':
    case 'x':
    case 'X':
    case 'o':
    case 'u':
    case 'd':
    case 'i':
      lg += sb.field_ - strlen(tmp);
      ch = (sb.flag_ & ZERO_PAD ? '0' : ' ');

      if (lg > 0 && !(sb.flag_ & LEFT_ADJ)) {
        while (lg--) {
          if (fp->write_(fp, &ch, 1) < 0)
            return -1;
        }
      }

      if (sb.flag_ & ALT_FORM && (str[-1] & 32) == 'x') {
        if (fp->write_(fp, "0x", 2) < 0)
          return -1;
      }

      if (fp->write_(fp, tmp, strlen(tmp)) < 0)
        return -1;

      if (lg > 0) {
        while (lg--) {
          if (fp->write_(fp, &ch, 1) < 0)
            return -1;
        }
      }

      break;
    }

  }

  FUNLOCK(fp);
  return fp->count_;
}

