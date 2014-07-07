#include <kinfo.h>

// ----------------------------------------------------------------------------
/**
    Allocate and copy a string
    The string returned can be freed using kfree
 */
char* kcopystr (const char* str)
{
  char* ptr;
  int lg;
  lg = strlen(str);

  if (lg > MAX_STRING_LENGTH) {
    __seterrno(ENOMEM);
    return NULL;
  }

  ptr = (char*)kalloc(lg + 1);
  memcpy(ptr, str, lg);
  ptr [lg] = '\0';
  return ptr;
}

// ============================================================================
#ifdef __KERNEL

#undef kalloc
#undef kfree

// ----------------------------------------------------------------------------
/**
    Allocate a block of memory on kernel heap.
    The memory is initialized to zero.
    The block of memory can be freed using kfree
 */
void* kalloc (size_t size)
{
  void* addr = malloc_r(&kSYS.kheap, size);

  if (addr == 0) {
    __seterrno(ENOMEM);
    kpanic ("The kernel run out of memory\n");
  }

  memset(addr, 0, size);
  return addr;
}

// ----------------------------------------------------------------------------
/**
    Free a memory block allocate on kernel heap
 */
void kfree (void* addr)
{
  free_r (&kSYS.kheap, addr);
  // memcorrupt_r (&kSYS.kheap);
}

#endif /* __KERNEL */
