#include <smkos/kernel.h>
#include <smkos/klimits.h>
#include <smkos/alloc.h>
// #include <smkos/_spec.h>
#include <stdlib.h>
#include <ctype.h>

/* ----------------------------------------------------------------------- */
int *_geterrno()
{
  return &kCPU.errno_;
}


/* ----------------------------------------------------------------------- */
int *__lockcounter()
{
  return &kCPU.lockCounter_;
}


/* ----------------------------------------------------------------------- */
void *kalloc (size_t size)
{
  void *addr = malloc_(size);

  if (addr == 0) {
    // __seterrno(ENOMEM);
    kpanic ("The kernel run out of memory\n");
  }

  // kSYS.objMemory_ += size;
  memset(addr, 0, size);
  return addr;
}


/* ----------------------------------------------------------------------- */
void kfree (void *addr)
{
  // size_t size = ((int*)addr)[-4];
  // kSYS.objMemory_ -= size;
  free_(addr);
}


/* ----------------------------------------------------------------------- */
#ifndef __AX_STR_EX
char *strdup (const char *str)
{
  char *ptr;
  int lg = strlen(str);

  if (lg >= STRING_MAX)
    return __seterrnoN(EINVAL, char);

  ptr = (char *)kalloc(lg + 1);
  if (!ptr)
    return NULL;

  memcpy(ptr, str, lg);
  ptr [lg] = '\0';
  return ptr;
}


/* ----------------------------------------------------------------------- */

/* Compare two null-terminated char strings */
int strcmpi (const char* str1, const char* str2)
{
  while ( *str1 && ( tolower(*str1) == tolower(*str2) ) ) {
      ++str1;
      ++str2;
  }

  return tolower(*str1) - tolower(*str2);
}


/* ----------------------------------------------------------------------- */

time_t time(time_t* ptime)
{
  time_t now = 0;

  if (ptime)
    *ptime = now;
  return now;
}


/* ----------------------------------------------------------------------- */
clock_t clock()
{
  clock_t ticks = 0;
  return ticks;
}
#endif



/* ----------------------------------------------------------------------- */

void __assert_do (int as, const char *ex, const char *at)
{
  if (!as)
    kpanic("Assertion: %s at %s.\n", ex, at);
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

