#include <string.h>

int write(int fd, const void *buf, unsigned int count);
int read(int fd, const void *buf, unsigned int count);
int open(const char *path, int flag, int mode);

typedef struct axKernelInfo axKernelInfo_t;
struct axKernelInfo {
  char    kernel_[8];
};

int sname (int name, ...);

int main (int argc, char **argv)
{
  axKernelInfo_t info;
  sname (0, &info);
  write (1, "Smoke-krn | devmach01 | core#0 | 0.0.0 | Jul 26 2014 | i386 | i486 | SmokeOS\n", 77);


  char buff [1024];
  int sc = open ("/master.log", 0, 0); // O_RDONLY
  int lg = read (sc, buff, 1024);

  if (lg > 0) {
    write (1, buff, lg);
  }

  write (1, "---\n", 4);
  memset (buff, 0, 1024);

  sc = open ("/dev/pts/p1", 0, 0); // O_RDONLY
  lg = read (sc, buff, 1024);

  if (lg > 0) {
    // write (1, buff, lg);
  }

  return 0;
}
