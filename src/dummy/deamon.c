


int write(int fd, const void *buf, unsigned int count);

int main () 
{
  write (0, "Date deamon v1.0\n", 17);

  for (;;) {
    int k, i = 0x8000000;
    while (--i > 0) {
      k  = (k | (i >> 3)) * 0x75f;
    }

    write (0, "deamon] Still here\n", 19);

  }

  return 0;
}
