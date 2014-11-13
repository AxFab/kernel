#include <smoke/syscall.h>

int syscall_1A(void* a1, int no);
int syscall_2A(void* a1, void* a2, int no);
int syscall_3A(void* a1, void* a2, void* a3, int no);
int syscall_4A(void* a1, void* a2, void* a3, void* a4, int no);

#define SYSCALL1(n,a1)        syscall_1A((void*)a1, n);
#define SYSCALL2(n,a1,a2)     syscall_2A((void*)a1, (void*)a2, n);
#define SYSCALL3(n,a1,a2,a3)  syscall_3A((void*)a1, (void*)a2, (void*)a3, n);



int strlen(const char* str)
{
  int i =0;
  while(*(str++)) ++i;
  return i;
}

void _puts (const char* str) 
{
  write(1, str, strlen(str));
}

int itimer(int miliseconds) 
{
  return SYSCALL2(SYS_ITIMER, (void*)miliseconds, 0);
}

time_t time(time_t* now)
{
  return (time_t)SYSCALL1(SYS_TIME, now);
}

int wait_obj(int handle, int what, int flags) 
{
  return SYSCALL3(SYS_WAIT, handle, what, flags);
}


int main() 
{
  _puts ("\e[33mKT_ITIMER\e[0m] Starting\n");
  int timer_handle = itimer(1000);
  int cnt = 20;
  while (--cnt) {
    time_t now = time(NULL);
    _puts ("\e[33mKT_ITIMER\e[0m] Ticks!\n");
    wait_obj(timer_handle, 0, 0);
  }

  // Try sleep()
  _puts ("\e[33mKT_ITIMER\e[0m] Ending...\n");
  return 0;  
}
