



int write(int fd, const void *buf, unsigned int count);

int main () 
{
  write (0, "Kernel master v1.0\n", 19);

  for (;;) {
    int k, i = 0x8000000;
    while (--i > 0) {
      k  = (k | (i >> 3)) * 0x75f;
    }

    write (0, "master] Still here\n", 19);

  }

  // printf ("Kernel master v1.0\n");
  // chdir ("/mnt/OS_CORE/");
  
  // open (".syr");
  // read (".syr");
  // exec ("enguig")
  return 0;
}
