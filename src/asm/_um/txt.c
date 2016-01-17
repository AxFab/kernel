#include <smkos/kernel.h>
#include <smkos/kstruct/user.h>
#include <stdio.h>

/* ----------------------------------------------------------------------- */
int kwrite_tty(const void *m, int lg)
{
  if (lg < 0)
    return printf ("%s", (const void*)m);
  else
    return fwrite((const void*)m, lg, 1, stdout);
}


/* ----------------------------------------------------------------------- */
void event_tty(int type, int value)
{
  __unused(type);
  __unused(value);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
