#ifndef SMOKE_SYSCALL_H__
#define SMOKE_SYSCALL_H__

#include <sys/types.h>

#define SYS_REBOOT  0
#define SYS_PAUSE  1

#define SYS_YIELD 0x08
#define SYS_TIME 0x09
#define SYS_SLEEP 0x0a
#define SYS_ITIMER 0x0b


#define SYS_EXIT    0x10
#define SYS_EXEC    0x11

#define SYS_WAIT 0x14

#define SYS_CLOSE   0x20
#define SYS_OPEN    0x21
#define SYS_READ    0x22
#define SYS_WRITE   0x23








#ifndef NULL
#  define NULL ((void*)0)
#endif



int syscall_1A(void *a1, int no);
int syscall_2A(void *a1, void *a2, int no);
int syscall_3A(void *a1, void *a2, void *a3, int no);
int syscall_4A(void *a1, void *a2, void *a3, void *a4, int no);

#define SYSCALL1(n,a1)        syscall_1A((void*)(a1), n);
#define SYSCALL2(n,a1,a2)     syscall_2A((void*)(a1), (void*)(a2), n);
#define SYSCALL3(n,a1,a2,a3)  syscall_3A((void*)(a1), (void*)(a2), (void*)(a3), n);


// ===========================================================================


typedef struct sStartInfo sStartInfo_t;

struct sStartInfo {
  const char     *cmd_;
  const char     *username_;
  int             output_;
  int             input_;
  int             error_;
  int             workingDir_;  ///
  int             flags_;       ///
  int             mainWindow_;  /// Give a window/tty handler for the program
};

int exec (const char *path, sStartInfo_t *param);
int execv_s(const char *path, const char *args);


int open(const char *file, int flags, ...);
int close(int fd);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);



#define ASCII_RESET   "\e[0m"
#define ASCII_U_WHITE "\e[53,38m"
#define ASCII_YELLOW  "\e[93m"
#define ASCII_RED     "\e[91m"
#define ASCII_BLUE_ON_WHITE   "\e[48,94m"

#endif /* SMOKE_SYSCALL_H__ */
