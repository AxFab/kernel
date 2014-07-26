#include <memory.h>

/** Check if this buffer cover a valid user space area
 *  \note the current requierment is to write on a single memory bucket
 */
int kUserParam_Buffer (kAddSpace_t* addp, void* base, size_t length)
{
  kVma_t* start = kVma_FindAt (addp, (uintptr_t)base);

  if (start == NULL)
    return 0;

  return ((uintptr_t)base + length) < start->limit_;
}

