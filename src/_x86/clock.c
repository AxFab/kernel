#include <smkos/kernel.h>
#include <smkos/klimits.h>
#include <time.h>

struct tm RTC_GetTime();
void PIT_Initialize (uint32_t frequency);


/* ----------------------------------------------------------------------- */
struct tm cpu_get_clock() {
  return RTC_GetTime();
}


/* ----------------------------------------------------------------------- */
int cpu_ticks_interval ()
{
  PIT_Initialize(CLOCK_HZ);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
int cpu_ticks_delay()
{
  return __seterrno(ENOSYS);
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
