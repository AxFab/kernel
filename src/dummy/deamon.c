

#define NULL ((void*)0)
typedef struct axStartInfo axStartInfo_t;


int write(int fd, const void *buf, unsigned int count);
int exec (const char *path, axStartInfo_t* param);


struct axStartInfo
{
  const char*     cmd_;
  const char*     username_;
  int             output_;
  int             input_;
  int             error_;
  int             workingDir_;  ///
  int             flags_;       ///
  int             mainWindow_;  /// Give a window/tty handler for the program
};



const char* cmdl[] = {
  NULL,
  "",
  "you",
  "babe! /tmp/hey.txt",

};


int execv_s(const char *path, const char * args)
{
  axStartInfo_t param = {args, NULL, 0, 0, 0, 0, 0, 0};
  return exec (path, &param);
}

void delay()
{
  int volatile k = 0xdeadbeaf;
  int volatile i = 0x8000000;
  while (--i > 0) {
    k  = (k | (i >> 3)) * 0x75f;
  }
}


int main ()
{
  int j;
  int o = 0;
  int m = 4; //sizeof (cmdl) / sizeof (char*);

  write (1, "Deamon v1.0\n", 17);

  for (;;) {

    for (j = 0; j < 2; ++j) {
      delay();
      write (1, "deamon - still here\n", 19);
    }

    delay();
    execv_s ("HELLO.", cmdl[o % m]);
    ++o;
  }

  return 0;
}
