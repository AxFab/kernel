#pragma once
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <smkos/scall.h>

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

void _puts(const char *str);

