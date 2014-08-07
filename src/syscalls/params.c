#include <kernel/memory.h>

/** Check if this buffer cover a valid user space area
 *  \note the current requierment is to write on a single memory bucket
 */
int kUserParam_Buffer (kAddSpace_t* addp, void* base, size_t length)
{
#ifdef __KERNEL
  kVma_t* start = kvma_look_at (addp, (uintptr_t)base);
  if (start == NULL) {
    __seterrno (EFAULT);
    return 0;
  }

  if (((uintptr_t)base + length) > start->limit_) {
    __seterrno (EFAULT);
    return 0;
  }
#endif
  return !0;
}

int kUserParam_String (kAddSpace_t* addp, const char* str, size_t max)
{
#ifdef __KERNEL
  kVma_t* start = kvma_look_at (addp, (uintptr_t)str);
  if (start == NULL) {
    __seterrno (EFAULT);
    return 0;
  }

  if (strnlen (str, max) >= max) {
    __seterrno (ENAMETOOLONG);
    return 0;
  }
#endif
  return !0;
}

