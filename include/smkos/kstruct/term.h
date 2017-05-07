#pragma once
#include <smkos/kernel.h>


/* If set, indicate that what we write on output is in fact user input. */
#define TTY_ON_INPUT  (1 << 0)
/* If set, indicate that we have content into input buffer. */
#define TTY_NEW_INPUT (1 << 1)
/* Ask to recho the line the next time the program read. */
#define TTY_READ_ECHO (1 << 2)

#define TTY_KEY_CTRL (1 << 4)
#define TTY_KEY_ALT (1 << 5)
#define TTY_KEY_SHIFT (1 << 6)




struct kLine {
  uint32_t txColor_;
  uint32_t bgColor_;
  int offset_;
  int flags_;
  kLine_t *prev_;
  kLine_t *next_;
};

struct kTerm {
  uint32_t txColor_;
  uint32_t bgColor_;

  kLine_t *first_;
  kLine_t *top_;
  kLine_t *last_;

  int width_;
  int height_;
  int line_;
  void *pixels_;
  kMemArea_t* pxArea_;

  int row_;
  int flags_;
  int max_row_;
  int max_col_;
  int colMax_;

  kPipe_t *pipe_;
  kInode_t* ino_;

  void (*paint) (kTerm_t *term, kLine_t *style, int ch, int row, int col);
  void (*clear)(kTerm_t *term);
};



struct bitmap_font {
  int width_;
  int height_;
  int dispx_;
  int dispy_;
  int glyph_size_;
  const uint8_t *const glyph_;
};

extern struct bitmap_font *fontbmp;
