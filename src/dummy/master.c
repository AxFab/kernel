

int sys_write(int fd, const void *buf, unsigned int count, int off);

int main ()
{
  sys_write (0, "Kernel master v1.0\n", 19, -1);

  for (;;) {
    int k, i = 0x8000000;
    while (--i > 0) {
      k  = (k | (i >> 3)) * 0x75f;
    }

    sys_write (0, "Master - still here\n", 19, -1);
  }

  // printf ("Kernel master v1.0\n");
  // chdir ("/mnt/OS_CORE/");

  // open (".syr");
  // read (".syr");
  // exec ("enguig")
  return 0;
}
