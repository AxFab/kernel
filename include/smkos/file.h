#pragma once
#include <smkos/assert.h>
#include <smkos/compiler.h>
#include <stddef.h>
#include <stdint.h>

#define EOF (-1)
#define FLOCK(f) ((void)fp)
#define FUNLOCK(f) ((void)fp)


typedef struct SMK_File FILE;

struct SMK_File {
  int     fd_;
  int     oflags_;
  FILE   *next_;

  int     lock_;
  int     count_;
  char    lbuf_;

  char   *buf_;
  size_t  off_;
  size_t  lpos_;
  size_t  bsize_;

  char   *rpos_;
  char   *rend_;

  char   *wpos_;
  char   *wend_;

  int (*write_) (FILE *restrict fp, const char *restrict buf, size_t length);
  int (*read_) (FILE *restrict fp, char *restrict buf, size_t length);
};


union SMK_FmtArg {
  void *p;
  int s;
  unsigned int i;
  double f;
};

struct SMK_FmtSpec {
  int flag_;
  int type_;
  int precis_;
  int field_;
};

uintmax_t _strtox(const char *str, char **endptr, int base, char *sign);
char *_utoa (uintmax_t value, char *str, int base, const char *digits);

