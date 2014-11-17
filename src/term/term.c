#include "kernel/term.h"

#define TERM_OUT_BUFFER 8192
#define TERM_IN_BUFFER 4096

// ---------------------------------------------------------------------------
kTerm_t* term_open (kInode_t* ino) 
{
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
  return term;
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
void term_scroll (kTerm_t* term, int count)
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


// ---------------------------------------------------------------------------
void term_redraw(kTerm_t* term)
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
// ---------------------------------------------------------------------------
