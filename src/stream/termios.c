#include <kernel/stream.h>
#include <kernel/async.h>
#include <kernel/keys.h>
#include <kernel/vfs.h>
#include <kernel/info.h>
#include <kernel/term.h>


int font64_paint(kTerm_t* term, kLine_t* style, int row);
void font64_clean(kTerm_t* term);


// ===========================================================================
static void term_shortcut (kStream_t* stm, int level, int ch) 
{
  static int shortcut[2][0x80] = {
    { // CTRL + 
      ['d'] = 0, // Signal: end of file
      ['c'] = 0, // Signal: abort
      ['w'] = 0, // Input: erase word to left
      ['u'] = 0, // Input: erase line to left
      ['k'] = 0, // Input: erase line to right
    },
    { // ALT + 
      ['d'] = 0, // Input: erase word to right
    },
  };

  __nounused(stm);
  __nounused(shortcut);
  assert (ch > 0 && ch < 0x80);
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
    // kprintf ("STDIN GOT A NEW LINE [%s - %d]\n", stm->ino_->name_, stm->ino_->evList_.count_);
    async_trigger (&stm->ino_->evList_, EV_READ, 0); // param LESS OR EQUALS TO avail or zero

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

static void term_keydown (kStream_t* stm, int key) 
{
  kTerm_t* term = stm->ino_->term_;
  switch (key) {
    case KEY_CTRL:
      term->flags_ |= TTY_KEY_CTRL;
      break;
    case KEY_ALT:
      term->flags_ |= TTY_KEY_ALT;
      break;
    case KEY_SH_LF:
    case KEY_SH_RG:
      term->flags_ |= TTY_KEY_SHIFT;
      break;
    case KEY_PAGE_UP:
      term_scroll(term, - term->max_row_ / 2);
      break;
    case KEY_PAGE_DW:
      term_scroll(term, term->max_row_ / 2);
      break;
    default:
      if (key >= 0x80)
        break;
      if (term->flags_ & TTY_KEY_CTRL)
        term_shortcut(stm, 0, key);
      else if (term->flags_ & TTY_KEY_ALT)
        term_shortcut(stm, 1, key);
      else
        term_input(stm, key);
      return;
  }
}

static void term_keyup (kStream_t* stm, int key) 
{
  kTerm_t* term = stm->ino_->term_;
  switch (key) {
    case KEY_CTRL:
      term->flags_ &= ~TTY_KEY_CTRL;
      break;
    case KEY_ALT:
      term->flags_ &= ~TTY_KEY_ALT;
      break;
    case KEY_SH_LF:
    case KEY_SH_RG:
      term->flags_ &= ~TTY_KEY_SHIFT;
      break;
    default:
      return;
  }
}

int term_event (kStream_t* stm, kEvent_t* event)
{
  assert (stm != NULL);
  assert (event != NULL);
  kTerm_t* term = stm->ino_->term_;
  int param2 = event->keyboard_.key_;
  assert (param2 > 0 && param2 < 0x80);
  param2 = (int)key_layout_us[param2][term->flags_ & TTY_KEY_SHIFT ? 1 : 0];

  switch (event->type_) {

    case EV_KEYDW:
      term_keydown (stm, param2);
      break;

    case EV_KEYUP:
      term_keyup (stm, param2);
      break;

    case EV_MOTION:
    case EV_MOUSEDW:
    case EV_MOUSEUP:
      break;
  }

  return 0;
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
  term->out_buf_ = kalloc(term->out_size_, 0);
  term->in_size_ = TERM_IN_BUFFER;
  term->in_buf_ = kalloc(term->in_size_, 0);
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
