

int write(int fd, const void *buf, unsigned int count);
int open(const char* path, int flag, int mode);

int main ()
{
  write (0, "Kernel master v1.0\n", 19);
  int sc = open ("/dev/vba", 7, 0);

  for (;;) {
    int k, i = 0x8000000;
    while (--i > 0) {
      k  = (k | (i >> 3)) * 0x75f;
    }

    write (0, "Master - still here\n", 19);

    int buf[] = {0xa61010, 0xa61010, 0xa61010, 0xa61010 };
    write (sc, buf, 16);

  }



  // printf ("Kernel master v1.0\n");
  // chdir ("/mnt/OS_CORE/");

  // open (".syr");
  // read (".syr");
  // exec ("enguig")
  return 0;
}
