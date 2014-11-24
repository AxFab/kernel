#include <kernel/stream.h>
#include <kernel/async.h>
#include <kernel/keys.h>
#include <kernel/vfs.h>
#include <kernel/info.h>


int font64_paint(kTerm_t* term, kLine_t* style, int row);
void font64_clean(kTerm_t* term);


// If set, indicate that what we write on output is in fact user input.
#define TTY_ON_INPUT  (1 << 0)  
// If set, indicate that we have content into input buffer.
#define TTY_NEW_INPUT (1 << 1)
// Ask to recho the line the next time the program read.
#define TTY_READ_ECHO (1 << 2)

#define TTY_KEY_CTRL (1 << 3)
#define TTY_KEY_ALT (1 << 4)
#define TTY_KEY_SHIFT (1 << 5)



#define TERM_OUT_BUFFER 8192
#define TERM_IN_BUFFER 4096


// ===========================================================================

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

enum {
  EV_NONE = 0,
  EV_KEYDW,
  EV_KEYUP,
  EV_MOTION,
  EV_MOUSEDW,
  EV_MOUSEUP,
};


// ===========================================================================
static void term_shortcut_ctrl (kStream_t* stm, int ch) 
{
  __nounused(stm);
  assert (ch > 0 && ch < 0x80);
  switch (ch) {
      case 'd': 
        // Signal: end of file
        break;

      case 'c': 
        // Signal: abort
        break;

      case 'w': 
        // Input: erase word to left
        break;

      case 'u': 
        // Input: erase line to left
        break;

      case 'k': 
        // Input: erase line to right
        break;
  }
}


// ---------------------------------------------------------------------------
static void term_shortcut_alt (kStream_t* stm, int ch) 
{
  __nounused(stm);
  assert (ch > 0 && ch < 0x80);
  switch (ch) {
      case 'd': 
        // Input: erase word to right
        break;        
  }
}


// ---------------------------------------------------------------------------
static void term_input (kStream_t* stm, int ch) 
{
  kTerm_t* term = stm->ino_->term_;
  assert (ch > 0 && ch < 0x80);

  term->flags_ |= TTY_ON_INPUT;
  term_write (stm, &ch, 1);
  // @todo save on input
  term->in_buf_[term->in_write_pen_] = ch;
  ++term->in_write_pen_;

  term->flags_ = (term->flags_ | TTY_NEW_INPUT) & ~TTY_ON_INPUT;

  if (ch == '\n') {
    // @todo wake up tasks waiting for read inode event!
  }
}


// ---------------------------------------------------------------------------
static void term_redraw(kTerm_t* term)
{
  int i;
  int pen;
  term->clear(term);
  kLine_t* start = term->top_;
  for (i = 1; i <= term->max_row_; ++i) {
    kLine_t line = *start; 
    pen = term->paint(term, &line, i);
    if (pen == 0)
      break;

    start = start->next_;
  }
}

// ---------------------------------------------------------------------------
static void term_scroll (kTerm_t* term, int count)
{
  if (count > 0) {
    while (count != 0) {
      if (term->top_->next_ == NULL)
        break;
      term->top_ = term->top_->next_;
      count--;
      term->row_--;
    }

  } else {
    while (count != 0) {
      if (term->top_->prev_ == NULL)
        break;
      term->top_ = term->top_->prev_;
      count++;
      term->row_++;
    }
  }

  term_redraw (term);
}



// ===========================================================================
/**
  */
ssize_t term_read(kStream_t* stm, void* buf, size_t count)
{
  ssize_t bytes = 0;
  kTerm_t* term = stm->ino_->term_;
  while (count > 0) {
    // if (term->inPen_ >= term->length_)
    //   term->inPen_ = 0;

    ssize_t cap = term->in_size_ - term->in_pen_;

    // We try to reach the end of a line 
    ssize_t nextEndl = 0;
    while (term->in_buf_[term->in_pen_ + nextEndl] != '\n' && 
        term->in_buf_[term->in_pen_ + nextEndl] != '\0' && 
        term->in_pen_ + nextEndl < term->in_size_) ++nextEndl;

    // FLAG ON INO THAT SAY READ ONLY IF WE HAVE A '\n'
    if (term->in_pen_ + nextEndl >= term->in_size_ || 
        term->in_buf_[term->in_pen_ + nextEndl] != '\n')
      return bytes;

    ++nextEndl;
    cap = MIN (cap, nextEndl);
    cap = MIN (cap, (ssize_t)count);
    if (cap == 0) 
      return bytes;
    
    memcpy (buf, &term->in_buf_[term->in_pen_], cap);
    count -= cap;
    term->in_pen_ += cap;
    bytes += cap;
    buf = ((char*)buf) + cap;

    // FLAG ON INO THAT SAY READ UNTIL '\n' AND STOP AFTER!
    if (((char*)buf)[-1] == '\n') 
      break;
  }

  // If we need, re echo the input line
  if (term->flags_ & TTY_READ_ECHO) {
    term->flags_ &= ~TTY_READ_ECHO;
    term->flags_ |= TTY_ON_INPUT;
    term_write (stm, &((char*)buf)[-bytes], bytes);
    term->flags_ &= ~TTY_ON_INPUT;
  }

  // Check if we have more on stock
  if (term->in_pen_ < term->in_write_pen_)
    term->flags_ |= TTY_NEW_INPUT; 
  else 
    term->flags_ &= ~TTY_NEW_INPUT; 

  return bytes;
}


// ---------------------------------------------------------------------------
/** 
  */
ssize_t term_write(kStream_t* stm, const void* buf, size_t count)
{
  int pen;
  ssize_t bytes = 0;
  kTerm_t* term = stm->ino_->term_;
  // ssize_t count = kstm_write_pipe(term->output_, buf, length);
  while (count > 0) {
    // @todo, handle a realy fat line, then a line larger than the buffer !
    if (term->out_pen_ >= term->out_size_)
      term->out_pen_ = 0; 
      // In this case, we write some stuff, but we have to destroy all kLine_t that are <


    ssize_t cap = term->out_size_ - term->out_pen_;
    cap = MIN (cap, (ssize_t)count);

    if (cap == 0) 
      break;

    memcpy (&term->out_buf_[term->out_pen_], buf, cap);
    count -= cap;
    term->out_pen_ += cap;
    bytes += cap;
    buf = ((char*)buf) + cap;
    // *((char*)buf) = '\0'; // FIXME segfault here
    // if (KLOG_RW) kprintf ("write #[%d;%d] {%s}\n", outCap, term->outPen_, "-");
  }
  
  for (;;) {

    if (term->row_ > term->max_row_) {
      term_scroll (term, term->row_ - term->max_row_);
    }

    pen = term->paint(term, term->last_, term->row_);

    if ((term->flags_ & (TTY_NEW_INPUT | TTY_ON_INPUT)) == TTY_NEW_INPUT)
      term->flags_ |= TTY_READ_ECHO;

    if (pen == 0)
      break;

    kLine_t* newLine =  KALLOC(kLine_t);
    newLine->offset_ = pen;
    newLine->txColor_ = term->last_->txColor_;
    newLine->bgColor_ = term->last_->bgColor_;
    newLine->flags_ = term->last_->flags_;
    term->last_->next_ = newLine;
    newLine->prev_ = term->last_;
    term->last_ = newLine;
    
    // @todo delete from first until offset > pen + term->maxCol_;

    term->row_++;
  }

  // export_bmp (term->width_, term->height_, term->pixels_);
  return bytes;
}


// ---------------------------------------------------------------------------
/** 
  * If we tape a character, we print it on output. And store it on input !
  * When we ask to read, we 
  */
unsigned char key_layout_us[128][4];
int term_event (kStream_t* stm, kEvent_t* event)
{
  kTerm_t* term = stm->ino_->term_;
  int param2 = event->keyboard_.key_;
  assert (param2 > 0 && param2 < 0x80);
  param2 = (int)key_layout_us[param2][term->flags_ & TTY_KEY_SHIFT ? 1 : 0];

  switch (event->type_) {

    case EV_KEYDW:

      // kprintf ("key down] %d - (%x) \n", param2, param2);
      switch (param2) {

        case KEY_SH_LF:
        case KEY_SH_RG:
          term->flags_ |= TTY_KEY_SHIFT;
          break;

        case KEY_CTRL:
          term->flags_ |= TTY_KEY_CTRL;
          break;

        case KEY_ALT:
          term->flags_ |= TTY_KEY_ALT;
          break;

        case KEY_PAGE_UP:
          term_scroll(term, - term->max_row_ / 2);
          break;

        case KEY_PAGE_DW:
          term_scroll(term, term->max_row_ / 2);
          break;

        // case KEY_TAB:
        //   b
        default:
          if (term->flags_ & TTY_KEY_CTRL) 
            term_shortcut_ctrl(stm, param2);
          else if (term->flags_ & TTY_KEY_ALT) 
            term_shortcut_alt(stm, param2);
          else 
            term_input(stm, param2);
          break;
      }
      break;

    case EV_KEYUP:
      switch (param2) {

        case KEY_SH_LF:
        case KEY_SH_RG:
          term->flags_ &= ~TTY_KEY_SHIFT;
          break;

        case KEY_CTRL:
          term->flags_ &= ~TTY_KEY_CTRL;
          break;

        case KEY_ALT:
          term->flags_ &= ~TTY_KEY_ALT;
          break;
      }
      break;

    case EV_MOTION:
    case EV_MOUSEDW:
    case EV_MOUSEUP:
      break;
  }

  return -1;
}



// ---------------------------------------------------------------------------
void term_frame(kTerm_t* term,
                void* pixels, 
                int width, 
                int height, 
                int line,
                int (*paint)(kTerm_t*, kLine_t*, int), 
                void (*clear)(kTerm_t*))
{
  assert (term != NULL);
  term->pixels_ = pixels;
  term->width_ = width;
  term->height_ = height;
  term->line_ = line;
  term->paint = paint;
  term->clear = clear;

  int fontW = 6;
  int fontH = 9;

  term->max_row_ = (height-1) / (fontH + 1);
  term->max_col_ = (width-2) / fontW;
}


// ---------------------------------------------------------------------------
void term_close (kTerm_t* term)
{
  while (term->first_) {
    kLine_t* l = term->first_->next_;
    kfree (term->first_);
    term->first_ = l;
  }

  kfree(term->out_buf_);
  kfree(term->in_buf_);
  kfree(term);
}


// ---------------------------------------------------------------------------
kInode_t* term_create (void* pixels, int width, int height, int line)
{
  char no[10];
  static int auto_incr = 0;

  snprintf (no, 10, "tty%d", auto_incr++);
  kInode_t* ino = create_inode(no, kSYS.pipeNd_, S_IFTTY | 0600, width * height);
  assert (ino != NULL);

  kTerm_t* term = KALLOC(kTerm_t);
  term->txColor_ = 0xffa6a6a6; // 0xff5c5c5c;
  term->bgColor_ = 0xff323232;
  term->out_size_ = TERM_OUT_BUFFER;
  term->out_buf_ = kalloc(term->out_size_);
  term->in_size_ = TERM_IN_BUFFER;
  term->in_buf_ = kalloc(term->in_size_);
  term->row_ = 1;
  term->first_ = KALLOC(kLine_t);
  term->first_->txColor_ = 0xffa6a6a6;
  term->first_->bgColor_ = 0xff323232;
  term->last_ = term->first_;
  term->top_ = term->first_;
  ino->term_ = term;

  term_frame (ino->term_, pixels, width, height, line, font64_paint, font64_clean);
  return ino;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
