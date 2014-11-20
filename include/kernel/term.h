#ifndef KERNEL_TERM_H_
#define KERNEL_TERM_H_

#include "kernel/vfs.h"



// If set, indicate that what we write on output is in fact user input.
#define TTY_ON_INPUT  (1 << 0)  
// If set, indicate that we have content into input buffer.
#define TTY_NEW_INPUT (1 << 1)
// Ask to recho the line the next time the program read.
#define TTY_READ_ECHO (1 << 2)

#define TTY_KEY_CTRL (1 << 3)
#define TTY_KEY_ALT (1 << 4)
#define TTY_KEY_SHIFT (1 << 5)

enum {
  KEY_SHIFT_LF = 1,
  KEY_SHIFT_RG = 2,
  KEY_CTRL_LF = 3,
  KEY_CTRL_RG = 4,
  KEY_ALT_LF = 5,
  KEY_ALT_RG = 6,
  KEY_ARROW_UP = 8,
  KEY_ARROW_DW = 9,
};


enum {
  EV_NONE = 0,
  EV_KEYDW,
  EV_KEYUP,
  EV_MOTION,
  EV_MOUSEDW,
  EV_MOUSEUP,
};

typedef struct kLine kLine_t;

struct kLine
{
  uint32_t txColor_;
  uint32_t bgColor_;
  int offset_;
  int flags_;
  kLine_t* prev_;
  kLine_t* next_;
};

struct kTerm
{
  uint32_t txColor_;
  uint32_t bgColor_;
  
  kLine_t* first_;
  kLine_t* top_;
  kLine_t* last_;
  
  int width_;
  int height_;
  int line_;
  void* pixels_;

  int row_;
  int flags_;
  int max_row_;
  int max_col_;

  char* out_buf_;
  ssize_t out_size_;
  ssize_t out_pen_;

  char* in_buf_;
  ssize_t in_size_;
  ssize_t in_write_pen_;
  ssize_t in_pen_;

  int (*paint)(kTerm_t* term, kLine_t* style, int row);
  void (*clear)(kTerm_t* term);
};



kTerm_t* term_open();
void term_close(kTerm_t* term);
void term_scroll(kTerm_t* term, int count);
void term_redraw(kTerm_t* term);
void term_frame(kTerm_t* term, void* pixels, int width, int height, int line,
            int (*paint)(kTerm_t*, kLine_t*, int), void (*clear)(kTerm_t*));


ssize_t term_read(kInode_t* ino, void* buf, size_t count);
ssize_t term_write(kInode_t* ino, const void* buf, size_t count);
int term_event (kInode_t* ino, kEvent_t* event);

#endif /* KERNEL_TERM_H_ */
