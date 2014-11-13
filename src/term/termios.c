#include "kernel/term.h"


// ---------------------------------------------------------------------------
static void term_shortcut_ctrl (kInode_t* ino, int ch) 
{
  __nounused(ino);
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
static void term_shortcut_alt (kInode_t* ino, int ch) 
{
  __nounused(ino);
  assert (ch > 0 && ch < 0x80);
  switch (ch) {
      case 'd': 
        // Input: erase word to right
        break;        
  }
}


// ---------------------------------------------------------------------------
static void term_input (kInode_t* ino, int ch) 
{
  assert (ch > 0 && ch < 0x80);

  ino->term_->flags_ |= TTY_ON_INPUT;
  term_write (ino, &ch, 1);
  // @todo save on input
  ino->term_->in_buf_[ino->term_->in_write_pen_] = ch;
  ++ino->term_->in_write_pen_;

  ino->term_->flags_ = (ino->term_->flags_ | TTY_NEW_INPUT) & ~TTY_ON_INPUT;
}

// ===========================================================================
/**
  */
ssize_t term_read(kInode_t* ino, void* buf, size_t count)
{
  ssize_t bytes = 0;
  kTerm_t* term = ino->term_;
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
    term_write (ino, &((char*)buf)[-bytes], bytes);
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
ssize_t term_write(kInode_t* ino, const void* buf, size_t count)
{
  int pen;
  ssize_t bytes = 0;
  kTerm_t* term = ino->term_;
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
int term_event (kInode_t* ino, int event, int param1, int param2)
{
  __nounused(param1);
  switch (event) {

    case EV_KEYDW:
      switch (param2) {

        case KEY_CTRL_LF:
        case KEY_CTRL_RG:
          ino->term_->flags_ |= TTY_KEY_CTRL;
          break;

        case KEY_ALT_LF:
        case KEY_ALT_RG:
          ino->term_->flags_ |= TTY_KEY_ALT;
          break;

        case KEY_ARROW_UP:
          term_scroll(ino->term_, - ino->term_->max_row_ / 2);
          break;

        case KEY_ARROW_DW:
          term_scroll(ino->term_, ino->term_->max_row_ / 2);
          break;

        // case KEY_TAB:
        //   break;

        default:
          if (ino->term_->flags_ & TTY_KEY_CTRL) 
            term_shortcut_ctrl(ino, param2);
          else if (ino->term_->flags_ & TTY_KEY_ALT) 
            term_shortcut_alt(ino, param2);
          else
            term_input(ino, param2);
          break;
      }
      break;

    case EV_KEYUP:
      switch (param2) {

        case KEY_CTRL_LF:
        case KEY_CTRL_RG:
          ino->term_->flags_ &= ~TTY_KEY_CTRL;
          break;

        case KEY_ALT_LF:
        case KEY_ALT_RG:
          ino->term_->flags_ &= ~TTY_KEY_ALT;
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
// ---------------------------------------------------------------------------
