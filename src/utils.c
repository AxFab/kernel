#include <smkos/kernel.h>
#include <smkos/_spec.h>
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
  
  kSYS.objMemory_ += size;
  memset(addr, 0, size);
  return addr;
}


/* ----------------------------------------------------------------------- */
void kfree (void *addr)
{
  size_t size = ((int*)addr)[-4];
  kSYS.objMemory_ -= size;
  free_(addr);
}


/* ----------------------------------------------------------------------- */
#ifndef strdup
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
#endif


/* ----------------------------------------------------------------------- */
#ifndef strcmpi
// Compare two null-terminated char strings
int strcmpi (const char* str1, const char* str2)
{
  while ( *str1 && ( tolower(*str1) == tolower(*str2) ) ) {
      ++str1;
      ++str2;
  }

  return tolower(*str1) - tolower(*str2);
}
#endif


/* ----------------------------------------------------------------------- */
#ifndef time
time_t time(time_t* ptime) 
{
  time_t now = 0;
  
  if (ptime)
    *ptime = now;
  return now;
}
#endif


/* ----------------------------------------------------------------------- */
#ifndef clock
clock_t clock() 
{
  clock_t ticks = 0;
  return ticks;
}
#endif


/* ----------------------------------------------------------------------- */
#ifndef kwrite
static uint16_t* txtBuffer = (uint16_t*)0xB8000;
static int txtIdx = 0;
void ascii_cmd(const char **m)
{
}

void kwrite(const char *m)
{
  for (; *m; ++m) {
    if (*m < 0x20) {
      if (*m == '\n') 
        txtIdx += 80 - (txtIdx % 80);
      else if (*m == '\t')
        txtIdx += 4 - (txtIdx % 4);
      else if (*m == '\e')
        ascii_cmd(&m);
      continue;
    }

    txtBuffer[txtIdx] = (*m & 0xff) | 0x700;
    txtIdx++;
  }
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

