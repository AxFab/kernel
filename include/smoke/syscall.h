#ifndef SMOKE_SYSCALL_H__
#define SMOKE_SYSCALL_H__

#include <sys/types.h>

#define SYS_REBOOT  0
#define SYS_EXIT    0x10
#define SYS_EXEC    0x11

#define SYS_CLOSE   0x20
#define SYS_OPEN    0x21
#define SYS_READ    0x22
#define SYS_WRITE   0x23



#define SYS_TIME 0x8
#define SYS_ITIMER 0x9
#define SYS_WAIT 0x14






#ifndef NULL
#  define NULL ((void*)0)
#endif

// ===========================================================================


typedef struct sStartInfo sStartInfo_t;

struct sStartInfo
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

int exec (const char *path, sStartInfo_t* param);
int execv_s(const char *path, const char * args);


int open(const char * file, int oflag, ...);
int close(int fd);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);






#endif /* SMOKE_SYSCALL_H__ */
