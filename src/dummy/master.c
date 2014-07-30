

int write(int fd, const void *buf, unsigned int count);
int open(const char* path, int flag, int mode);

int main ()
{
  const char * logs[] = {
     "master log...         \n",
     "nothing happened!     \n",
     "gosh I'm bored!       \n",
     "call me MASTER!       \n",
     "waiting for something?\n",
     "I'll better continue. \n",
  };

  write (0, "Kernel master v1.0\n", 19);
  int sc = open ("/master.log", 0x41, 0644); // O_WRONLY | O_CREAT

  int j=0;

  for (;;) {

    int k, i = 0x8000000;
    while (--i > 0) {
      k  = (k | (i >> 3)) * 0x75f;
    }

    write (0, "Master - still here\n", 20);

    // int buf[] = {0xa61010, 0xa61010, 0xa61010, 0xa61010 };

    write (sc, logs[j++ % 6], 23);
  }



  // printf ("Kernel master v1.0\n");
  // chdir ("/mnt/OS_CORE/");

  // open (".syr");
  // read (".syr");
  // exec ("enguig")
  return 0;
}
