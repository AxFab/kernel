#include <string.h>
#include <stdarg.h>
#include <smkos.h>

int vsnprintf(char *, int lg, const char *msg, va_list ap);


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
  sStartInfo_t param = {args, NULL, -1, -1, -1, -1, 0, -1};
  return exec (path, &param);
}

int errno;
int *_geterrno()
{
  return &errno;
}


int printf(const char *msg, ...)
{
  int res;
  char tmp[120];
  va_list ap;
  va_start(ap, msg);
  res = vsnprintf(tmp, 120, msg, ap);
  va_end (ap);
  return res;
}

