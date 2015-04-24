#include <smkos/kernel.h>
#include <smkos/core.h>
#include <smkos/arch.h>

#ifdef alloc_init
void alloc_init()
{
}
#endif

/* ----------------------------------------------------------------------- */
void mmu_prolog ()
{
}


/* ----------------------------------------------------------------------- */
/** Function to inform paging module that some RAM can be used by the system .
  * @note this function should be used before mmu_init after mmu_prolog.
  */
void mmu_ram (int64_t base, int64_t length)
{
}


/* ----------------------------------------------------------------------- */
int mmu_init ()
{
  return 0;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
