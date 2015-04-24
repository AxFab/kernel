#include <smkos.h>


int itimer(int miliseconds)
{
  return SYSCALL2(SYS_ITIMER, (void *)miliseconds, 0);
}

int wait_obj(int handle, int what, int flags)
{
  return SYSCALL3(SYS_WAIT, handle, what, flags);
}


int main()
{
  _puts ("\e[94mKT_ITIMER\e[0m] Starting\n");
  // int timer_handle = itimer(1000);
  int cnt = 20;

  while (--cnt) {
    // time_t now = time(NULL);
    _puts ("\e[94mKT_ITIMER\e[0m] Ticks!\n");
    //wait_obj((1000ULL * 1000ULL) * 3ULL /*timer_handle*/, 1, 0); // Nicroseconds
    sleep (3);
  }

  // Try sleep()
  _puts ("\e[94mKT_ITIMER\e[0m] Ending...\n");
  return 0;
}
