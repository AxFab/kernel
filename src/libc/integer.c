#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define LOWER 0x20

const char* _utoa_digits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+=";
const char* _utoa_digitsX = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+=";


// ---------------------------------------------------------------------------
uintmax_t _strtox(const char * str, char ** endptr, int base, char* sign)
{
  uintmax_t value = 0;

  if (endptr) (*endptr) = (char*)str;
  while (isspace(*str)) str++;

  (*sign) = *str == '-' ?  '-' : '+';
  if (*str == '-' || *str == '+') {
    str++;
  }

  if (base == 0) {
    if (str[0] != '0') {
      base = 10;

    } else if ((str[1] | LOWER) == 'x') {
      base = 16;
      str += 2;

    } else {
      base = 8;
      str++;
    }
  }

  if (*str < '0' || (*str | LOWER) >= _utoa_digits[base]) {
    // TODO errno !?
    return 0;
  }

  for (;;str++) {

    if (*str < '0' || (*str | LOWER) >= _utoa_digits[base])
      break;

    value *= base;

    if (*str <= '9')
      value += (*str) - '0';
    else
      value += ((*str) | LOWER) - 'a' + 10;

  }

  if (endptr) (*endptr) = (char*)str;
  return value;
}

// ---------------------------------------------------------------------------
int atoi (const char* str)
{
  char sign;
  uintmax_t value = _strtox(str, NULL, 10, &sign);
  return (int) (sign == '+') ? value : -value;
}

// ---------------------------------------------------------------------------
long atol (const char* str)
{
  char sign;
  uintmax_t value = _strtox(str, NULL, 10, &sign);
  return (long) (sign == '+') ? value : -value;
}

// ---------------------------------------------------------------------------
long long atoll (const char* str)
{
  char sign;
  uintmax_t value = _strtox(str, NULL, 10, &sign);
  return (long long) (sign == '+') ? value : -value;
}


// ---------------------------------------------------------------------------
long strtol (const char * str, char ** endptr, int base)
{
  char sign;
  uintmax_t value;

  if (base != 0 && (base < 2 || base > 36)) {
    return 0;
  }

  value = _strtox(str, endptr, base, &sign);

  if (sign == 'o') {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  if (sign == '+') {
    if (value > LONG_MAX) {
      errno = EOVERFLOW;
      if (endptr) (*endptr) = (char*)str;
      return 0;
    }

    return (long) value;

  } else {

    if (value > (-(uintmax_t)LONG_MIN)) {
      errno = EOVERFLOW;
      if (endptr) (*endptr) = (char*)str;
      return 0;
    }

    return (long) -value;
  }
}


// ---------------------------------------------------------------------------
long long strtoll (const char * str, char ** endptr, int base)
{
  char sign;
  uintmax_t value;

  if (base != 0 && (base < 2 || base > 36)) {
    return 0;
  }

  value = _strtox(str, endptr, base, &sign);

  if (sign == 'o') {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  if (sign == '+') {
    if (value > LLONG_MAX) {
      errno = EOVERFLOW;
      if (endptr) (*endptr) = (char*)str;
      return 0;
    }

    return (long long) value;

  } else {

    if (value > (-(uintmax_t)LLONG_MIN)) {
      errno = EOVERFLOW;
      if (endptr) (*endptr) = (char*)str;
      return 0;
    }

    return (long long) -value;
  }
}

// ---------------------------------------------------------------------------
unsigned long strtoul (const char * str, char ** endptr, int base)
{
  char sign;
  uintmax_t value;

  if (base != 0 && (base < 2 || base > 36)) {
    return 0;
  }

  value = _strtox(str, endptr, base, &sign);

  if (sign == 'o') {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  if (value > ULONG_MAX) {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  return (unsigned long) (sign == '+' ? value : -value);
}

// ---------------------------------------------------------------------------
unsigned long long strtoull (const char * str, char ** endptr, int base)
{
  char sign;
  uintmax_t value;

  if (base != 0 && (base < 2 || base > 36)) {
    return 0;
  }

  value = _strtox(str, endptr, base, &sign);

  if (sign == 'o') {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  if (value > ULLONG_MAX) {
    errno = EOVERFLOW;
    if (endptr) (*endptr) = (char*)str;
    return 0;
  }

  return (unsigned long long) (sign == '+' ? value : -value);
}

// ---------------------------------------------------------------------------

char* _utoa (uintmax_t value, char * str, int base, const char* digits)
{
  int sp = 0, j=0;
  char stack[sizeof(uintmax_t)*8] = { 0 };

  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return 0;
  }

  while (value) {
    lldiv_t res = lldiv(value, base);
    stack[sp++] = digits[res.rem];
    value = res.quot;
  }

  str [sp] = '\0';
  while (sp > 0) {
    --sp;
    str [j++] = stack[sp];
  }

  return str;
}


char* itoa (int value, char * str, int base)
{
  char* ptr = str;
  if (base == 10 && value < 0) {
    *(str++) = '-';
    value = -value;
  }

  _utoa (value, str, base, _utoa_digits);
  return ptr;
}



