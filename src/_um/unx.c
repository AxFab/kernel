#include <smkos/kernel.h>
#include <smkos/kapi.h>

#include <stdlib.h>
#include <stdio.h>

/* ----------------------------------------------------------------------- */
void alloc_init()
{
}

/* ----------------------------------------------------------------------- */
void *malloc_ (size_t size)
{
  return malloc(size);
}


/* ----------------------------------------------------------------------- */
void free_ (void *addr)
{
  free(addr);
}

/* ----------------------------------------------------------------------- */
void VGA_Info()
{
}

void kwrite(const char *m)
{
  printf("%s", m);
}
