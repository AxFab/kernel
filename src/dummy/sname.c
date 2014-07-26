#include <string.h>

typedef struct axKernelInfo axKernelInfo_t;
struct axKernelInfo
{
  char    kernel_[8];
};

int sys_sname (int name, ...);

int main (int argc, char** argv)
{
  axKernelInfo_t info;
  sys_sname (0, &info);
  sys_write (1, "Smoke-krn | devmach01 | core#0 | 0.0.0 | Jul 26 2014 | i386 | i486 | SmokeOS\n", 77, -1);

  return 0;
}
