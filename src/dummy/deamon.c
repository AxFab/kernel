#include <smoke/syscall.h>

// #define NULL ((void*)0)
// typedef struct sStartInfo sStartInfo_t;


// int write(int fd, const void *buf, unsigned int count);
// int exec (const char *path, sStartInfo_t* param);


// struct sStartInfo
// {
//   const char*     cmd_;
//   const char*     username_;
//   int             output_;
//   int             input_;
//   int             error_;
//   int             workingDir_;  ///
//   int             flags_;       ///
//   int             mainWindow_;  /// Give a window/tty handler for the program
// };



const char *cmdl[] = {
  NULL,
  "",
  "you",
  "babe! /tmp/hey.txt",

};


// int execv_s(const char *path, const char *args)
// {
//   sStartInfo_t param = {args, NULL, 0, 0, 0, 0, 0, 0};
//   return exec (path, &param);
// }

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
  int m = 4; // sizeof (cmdl) / sizeof (char*);

  write (1, "Deamon v1.0\n", 12);

  execv_s ("KT_ITIME.XE", NULL);

  for (;;) {

    char tmp[512];

    write (1, "\e[93madmin@vm01:~\e[0m> ", 23);
    int lg = read (0, tmp, 500);

    if (lg > 0) {
      tmp[lg] = '\0';
      write (1, "\e[32mOK\e[0m\n", 12);
      write (1, tmp, lg);
    } else if (lg == 0) {
      write (1, "\e[43,30mZr\e[0m\n", 15);
    } else
      write (1, "\e[31mEr\e[0m\n", 12);


    // for (j = 0; j < 2; ++j) {
    //   delay();
    //   write (1, "deamon - still here\n", 20);
    // }

    delay();
    // execv_s ("HELLO.", cmdl[o % m]);
    // execv_s ("SNAME.", NULL);
    ++o;
  }

  return 0;
}
