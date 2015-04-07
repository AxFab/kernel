#include <smoke/syscall.h>



// Copy a null-terminated char string
char *strcpy ( char *dest, const char *src )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Copy a char string
char *strncpy ( char *dest, const char *src, size_t length )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( length-- > 0 && ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Concat two null-terminated char strings
char *strcat ( char *dest, const char *src )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( *ptr1 ) {
    ++ptr1;
  }

  while ( ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Concat two null-terminated char strings
char *strncat ( char *dest, const char *src, size_t length )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( *ptr1 ) {
    ++ptr1;
  }

  while ( length-- > 0 && ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Compare two null-terminated char strings
int strcmp ( const char *str1, const char *str2 )
{
  while ( *str1 && ( *str1 == *str2 ) ) {
    ++str1;
    ++str2;
  }

  return *str1 - *str2;
}

// Compare two char strings
int strncmp ( const char *str1, const char *str2, size_t length )
{
  while ( --length && *str1 && *str1 == *str2 ) {
    ++str1;
    ++str2;
  }

  return *str1 - *str2;
}


int strlen(const char *str)
{
  int i = 0;

  while (*(str++)) ++i;

  return i;
}

void _puts (const char *str)
{
  write(1, str, strlen(str));
}

void _delay ()
{
  int volatile k = 0xdeadbeaf;
  int volatile i = 0x8000000;

  while (--i > 0) {
    k  = (k | (i >> 3)) * 0x75f;
  }
}

time_t time(time_t *now)
{
  return (time_t)SYSCALL1(SYS_TIME, now);
}


int sleep (int seconds)
{
  return SYSCALL1(SYS_SLEEP, seconds * 1000);
}

int execv_s(const char *path, const char *args)
{
  sStartInfo_t param = {args, NULL, 0, 0, 0, 0, 0, 0};
  return exec (path, &param);
}

int errno;
int *_geterrno()
{
  return &errno;
}


void printf(const char *msg, ...)
{
  char tmp[120];
  void *ap = ((&msg) + sizeof(void *));
  vsnprintf(tmp, 120, msg, ap);
}

