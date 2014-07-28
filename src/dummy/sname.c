#include <string.h>

int write(int fd, const void *buf, unsigned int count);

typedef struct axKernelInfo axKernelInfo_t;
struct axKernelInfo
{
  char    kernel_[8];
};

int sname (int name, ...);

int main (int argc, char** argv)
{
  axKernelInfo_t info;
  sname (0, &info);
  write (1, "Smoke-krn | devmach01 | core#0 | 0.0.0 | Jul 26 2014 | i386 | i486 | SmokeOS\n", 77);

  return 0;
}
