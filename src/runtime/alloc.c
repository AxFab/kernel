#include <kernel/core.h>

// ----------------------------------------------------------------------------
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

// ============================================================================

// FIXME Why here !?
time_t time (time_t *ptr)
{
  return (time_t)(kSYS.now_ / 1000000);
}

#undef kalloc
#undef kfree

// ----------------------------------------------------------------------------
/**
    Allocate a block of memory on kernel heap.
    The memory is initialized to zero.
    The block of memory can be freed using kfree
 */
void *kalloc (size_t size, int slab)
{
  void *addr = malloc_r(&kSYS.kheap, size);

  if (addr == 0) {
    __seterrno(ENOMEM);
    kprintf ("ENOMEM on %s [%d] %x\n", kpsize(size), slab, &kSYS);
    kpanic ("The kernel run out of memory\n");
  }

  memset(addr, 0, size);
  return addr;
}

// ----------------------------------------------------------------------------
/**
    Free a memory block allocate on kernel heap
 */
void kfree (void *addr)
{
  free_r (&kSYS.kheap, addr);
  // memcorrupt_r (&kSYS.kheap);
}

// #endif /* __KERNEL */
