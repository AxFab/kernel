#include <smkos/kernel.h>
#include <smkos/kstruct/user.h>
#include <stdio.h>

/* ----------------------------------------------------------------------- */
void kwrite_tty(const char *m)
{
  printf ("%s", m);
}


/* ----------------------------------------------------------------------- */
void event_tty(int type, int value)
{
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */