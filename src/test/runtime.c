#include <kernel/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <check.h>

/**
    Change the kernel error status.
    On debug/paranoid mode, each error are logged.
 */
int kseterrno(int err, const char *at)
{
  if (err) {
    fprintf(stderr, "Error %d at %s [%s]\n", err, at, strerror(err));
    // kstacktrace (8);
  }

  kCPU.errNo = err;
  return kCPU.errNo;
}

int kgeterrno()
{
  return kCPU.errNo;
}

int kprintf(const char *str, ...)
{
  va_list ap;
  va_start(ap, str);
  int sz = vfprintf(stderr, str, ap);
  va_end(ap);
  return sz;
}

int kpanic(const char *str, ...)
{
  char tmp [512];
  va_list ap;
  va_start(ap, str);
  int sz = vsnprintf (tmp, 512, str, ap);
  va_end(ap);
  ck_assert_msg (!0, tmp);
  fprintf (stderr, "KERNEL PANIC - %s\n", str);
  return sz;
}

/**
    Allocate and copy a string
    The string returned can be freed using kfree
 */
char *kstrdup (const char *str)
{
  char *ptr;
  int lg;
  lg = strlen(str);

  if (lg > MAX_STRING_LENGTH) {
    __seterrno(ENOMEM);
    return NULL;
  }

  ptr = (char *)kalloc(lg + 1, 0);
  memcpy(ptr, str, lg);
  ptr [lg] = '\0';
  return ptr;
}

// ----------------------

bool mem_accounting = true;
int alloc_count = 0;
int alloc_size = 0;
int alloc_total = 0;
int alloc_pick = 0;
void *kalloc (size_t size, int slab)
{
  int upsize;
  __nounused(slab);
  void *ptr = calloc(size, 1);

  if (mem_accounting) {
    alloc_count++;
    upsize = ((int *)ptr)[-1] & ~0x7;
    alloc_size += upsize;
    alloc_total += upsize;

    if (alloc_size > alloc_pick) alloc_pick = alloc_size;

    printf("Trace] allocation of size %d [%d]  <%d block for %d bytes>\n", size, upsize, alloc_count, alloc_size);
  }

  return ptr;
}

void kfree (void *ptr)
{
  if (mem_accounting) {
    int size = ((int *)ptr)[-1] & ~0x7;
    alloc_count--;
    alloc_size -= size;
    printf("Trace] free pointer of size %d <%d block for %d bytes>\n", size, alloc_count, alloc_size);
  }

  free(ptr);
}

void kstat()
{
  if (mem_accounting)
    printf("Trace] Allocation is ::\n    Count \t%d\n    Size \t%d\n    Total \t%d\n    Pick \t%d\n",
           alloc_count, alloc_size, alloc_total, alloc_pick);
}

// int kpanic(const char* str, ...);

// int kputc(int c);
// int kprintf(const char* str, ...);
// int kvprintf (const char* str, va_list ap);
// const char* kpsize (uintmax_t number);


// void* kalloc(size_t size, int slab);
// void kfree(void* addr);
// char* kstrdup(const char* str);

// const char* ksymbol (void* address);
// void kstacktrace(uintptr_t max_frames);
// void kdump (void* ptr, size_t lg);
// void kregisters (kCpuRegs_t* regs);

int bclearbyte (uint8_t *byte, int off, int lg)
{
  uint8_t v = byte[0];
  int mask = (0xFF << off) & 0xFF;

  if (lg + off < 8) {
    mask = (mask & ~(0xFF << (off + lg))) & 0xFF;
  }

  byte[0] = v & ~mask;
  return (v ^ mask) & mask;
}


int bclearbytes (uint8_t *table, int offset, int length)
{
  int ox = offset / 8;
  int oy = offset % 8;
  int r = 0;

  if (oy != 0 || length < 8) {
    if (length + oy < 8) {
      r |= bclearbyte(&table[ox], oy, length);
      length = 0;
    } else {
      r |= bclearbyte(&table[ox], oy, 8 - oy);
      length -= 8 - oy;
    }

    ox++;
  }

  while (length >= 8) {
    r |= ~table[ox];
    table[ox] = 0;
    ox++;
    length -= 8;
  }

  if (length > 0) {
    r |= bclearbyte(&table[ox], 0, length);
  }

  return r;
}

kCpuCore_t kCPU;
kSysCore_t kSYS;


