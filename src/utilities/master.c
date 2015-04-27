
#include <smkos.h>

int read_registers (const char *file)
{
  return -1;
}

int auth_user (const char *username)
{
  char tmp [1000];
  strcpy (tmp, ASCII_BLUE_ON_WHITE "master] " ASCII_RED "unknow username '");
  strcat (tmp, username);
  strcat (tmp, "'!\n" ASCII_RESET);
  _puts (tmp);

  execv_s ("KT_ITIME.XE", NULL);
  return -1;
}

int start_shell (const char *username)
{
  sStartInfo_t sinfo;
  sinfo.cmd_ = "axsh -l";
  sinfo.username_ = username;
  // sinfo.flags_ = SI_LOGIN;
  exec ("/usr/bin/axsh", &sinfo);
  // execv_s > AXSH as username on username home...
  // wait for completion
  return 0;
}


int login ()
{
  int err = 0;
  char buffer [250];

  while (err < 3) {
    // _puts (ASCII_YELLOW "\nLogin :: " ASCII_RESET);
    _puts ("\n" ASCII_YELLOW "Login :: " ASCII_RESET);
    ssize_t sz = read (0, buffer, 250);

    if (sz < 0) {
      ++err;
      _puts ( ASCII_BLUE_ON_WHITE "master] " ASCII_RED "an error occurs unable to read username\n" ASCII_RESET);
      continue;
    }

    err = 0;

    if (buffer[sz - 1] == '\n')
      buffer[sz - 1] = '\0';
    else
      buffer[sz] = '\0';

    if (auth_user (buffer) != 0)
      continue;

    int cmd = start_shell (buffer);

    if (cmd == 0)
      continue;

    // cmd can be EXIT, OR A SPECIAL CLEAN COMMAND...
  }

  return -1;
}

int main ()
{
  _puts (ASCII_U_WHITE "Kernel master v1.0" ASCII_RESET "\n");

  // Open register
  // Start a new(1+) thread(s) for reg-service
  login ();

  // Login loop exited, shutdown...

  // const char * logs[] = {
  //    "master log...         \n",
  //    "nothing happened!     \n",
  //    "gosh I'm bored!       \n",
  //    "call me MASTER!       \n",
  //    "waiting for something?\n",
  //    "I'll better continue. \n",
  // };


  // int sc = open ("master.log", 0x43, 0644); // O_WRONLY | O_CREAT
  // if (sc != 3) {
  //   write (1, "The first file open on master is not 3!\n", 40);
  // }

  // if (sc == 0) {
  //   write (1, "The first file open on master is zero!!\n", 40);
  // }
  // int s1 = open ("/dev/fb0", 0x02, 0644); // O_WRONLY

  // int j=0;

  // execv_s ("DEAMON.", NULL);


  // for (;;) {

  //   // _delay();
  //   sleep (1);

  //   write (1, "\e[31mMaster\e[0m - still here\n", 29);

  //   write (sc, logs[j++ % 6], 23);

  // }



  // printf ("Kernel master v1.0\n");
  // chdir ("/mnt/OS_CORE/");

  // open (".syr");
  // read (".syr");
  // exec ("enguig")
  return 0;
}
