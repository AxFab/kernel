#include <smkos/kernel.h>
#include <smkos/kapi.h>

#include <stdlib.h>



uint8_t inb (uint16_t port) {
  return -1;
}
void outb (uint16_t port, uint8_t value) {}

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
void* valloc_(ssize_t size)
{
  return valloc(size);
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

