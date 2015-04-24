#include <smkos.h>

// void smk_run_thread(void(*entry)(long), long param)
// {
//   entry(param);
//   syscall_3(SYSC_STOP_THREAD, 0);
//   exit(-ESMK_SYSC_FAILED);
// }

// int smk_new_thread(void(*entry)(long), long param)
// {
//   return syscall_3(SYSC_START_THREAD, smk_run_thread, entry, param);
// }


// const char** devs = {
//   "dev/sdA",
//   "dev/sdB",
//   "dev/sdC",
//   "dev/sdD",
// }

// void run(long no)
// {
//   int fd = open(dev[no], O_RDONLY);
//   if (!fd) {
//     printf("This file cant be opened %s\n": dev[no]);
//     return;
//   }

//   unsigned char buf[512];
//   if (read(fd, buf, 512) != 512)
//     printf("Err. while reading file %s\n": dev[no]);

//   buf[510] = 0x55;
//   buf[511] = 0xAA;
//   buf[32] = 'A';
//   buf[33] = 'x';
//   buf[34] = 'F';
//   buf[35] = 'a';
//   buf[36] = 'b';
//   buf[37] = '\0';
//   // Compute Size -> /SYSTEM -> 20% /APPS -> 40% /USERS -> 40%

//   if (write(fd, buf, 512) != 512)
//     printf("Err. while writing on file %s\n": dev[no]);

//   // Mount
//   // Format FAT on all

//   smk_exec_command("installer.xe", dev[no]);
// }
typedef char bool;
#define true (!0)
#define false  (0)

int main ()
{
  int i, j;

  _puts(ASCII_U_WHITE "Master Program v0.1" ASCII_RESET "\n");
  sleep(2);

  // smk_new_thread(run, 0);
  // smk_new_thread(run, 1);
  // smk_new_thread(run, 2);
  // smk_new_thread(run, 3);

  int primes[500];
  int last = 1;

  for (i = 0; i < 500; ++i) {

    bool isprime = false;

    while (!isprime) {
      last += 2;
      isprime = true;

      for (j = 0; j < i; j++) {
        if (last % primes[j] == 0) {
          isprime = false;
          break;
        }
      }
    }

    primes[i] = last;

    printf("Find a new primes (%d), %d \n", i, primes[i]);
    sleep(1);
  }

  sleep(8);
  return 0;
}
