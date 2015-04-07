// #include <kernel/core.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_STRING_LENGTH 512

/* ----------------------------------------------------------------------- */
struct tm cpu_get_clock() {
  time_t now;
  struct tm *ptm;

  time(&now);
  ptm = gmtime(&now);
  return *ptm;
}


/* ----------------------------------------------------------------------- */
int cpu_ticks_interval ()
{
  PIT_Initialize(CLOCK_HZ);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
int cpu_ticks_delay()
{
  return __seterrno(ENOSYS);
}


/* ----------------------------------------------------------------------- */
void kpanic(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  vprintf(msg, ap);
  va_end(ap);
  exit (EXIT_FAILURE);
}

/* ----------------------------------------------------------------------- */
void kprintf(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  vprintf(msg, ap);
  va_end(ap);
}

/* ----------------------------------------------------------------------- */
static int __errno;
int *_geterrno()
{
  return &__errno;
}


/* ----------------------------------------------------------------------- */
void *kalloc (size_t size, int slab)
{

  void *addr = malloc(size);

  if (addr == 0) {
    // __seterrno(ENOMEM);
    kpanic ("The kernel run out of memory\n");
  }

  memset(addr, 0, size);
  return addr;
}


/* ----------------------------------------------------------------------- */
void kfree (void *addr)
{
  free (addr);
}


/* ----------------------------------------------------------------------- */
char *kstrdup (const char *str)
{
  char *ptr;
  int lg;
  lg = strlen(str);

  if (lg > MAX_STRING_LENGTH) {
    // __seterrno(ENOMEM);
    return NULL;
  }

  ptr = (char *)kalloc(lg + 1, 0);
  memcpy(ptr, str, lg);
  ptr [lg] = '\0';
  return ptr;
}

void _assert() {}
void __delay() {}
void inb() {}
void insl() {}
void insw() {}
void kCpu_Halt() {}
void kCpu_Switch() {}
void kCpu_Switch2() {}
void kCpu_SetStatus() {}
void keyboard_tty() {}
void klogLg() {}
void klogStart() {}
void kpsize() {}
void ksymreg() {}
void mmu_init() {}
void mmu_newpage() {}
void mmu_prolog() {}
void mmu_ram() {}
void mmu_reset_stack() {}
void mmu_resolve() {}
void mmu_temporary() {}
void outb() {}
void outsw() {}
void task_pause() {}
void throw() {}
int kCPU;
int kSYS;


/* ----------------------------------------------------------------------- */
int main ()
{
  kCore_Initialize ();

  return 0;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
