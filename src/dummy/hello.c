
int write(int fd, const void *buf, unsigned int count);
int open(const char* path, int flag, int mode);


void delay()
{
  int volatile k = 0xdeadbeaf;
  int volatile i = 0x8000000;
  while (--i > 0) {
    k  = (k | (i >> 3)) * 0x75f;
  }
}

int strhl (char *tmp, char* str)
{
  int i;
  char* h = "Hello ";

  for (i = 0; i < 6; ++i) {
    tmp[i] = h[i];
  }

  while (str[i-6]) {
    tmp[i] = str[i-6];
    ++i;
  }

  tmp[i++] = '\n';
  tmp[i] = '\0';
  return i;
}

int main (int argc, char** argv)
{
  char tmp[512];
  char* who = "world.";
  int where = 1;

  if (argc > 1) {
    who = argv[1];
  }

  if (argc > 2) {
    where = open (argv[2], /*O_WRONLY | O_APPEND | O_CREAT*/ 0, 0);
  }

  delay();

  int lg = strhl (tmp, who);
  write (where, tmp, lg);

  return 0;
}
