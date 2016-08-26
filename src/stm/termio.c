/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Implementation for terminals.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/user.h>
#include <smkos/kstruct/term.h>

#define _ESC 0x1B
#define _EOL '\n'

kInode_t *search_child2(const char *name, kInode_t *dir);
/* ----------------------------------------------------------------------- */
int kwrite_tty(const void *m, int lg);
int kwrite_pipe(const void *m, int lg);

void event_tty(int type, int value);
void event_pipe(int type, int value);


void font64_paint (kTerm_t *term, kLine_t *style, int ch, int row, int col);
void font64_clean(kTerm_t *term);


kSubSystem_t vgaText = {
#ifdef _MSC_BUILD
  INIT_MUTEX,
  event_tty, kwrite_tty,
  0, 0, 0
#else
  .write = kwrite_tty,
  .event = event_tty
#endif
};

kSubSystem_t frameTty = {
#ifdef _MSC_BUILD
  INIT_MUTEX,
  event_pipe, kwrite_pipe,
  0, 0, 0
#else
  .write = kwrite_pipe,
  .event = event_pipe,
#endif
};


kSubSystem_t *sysLogTty = &vgaText;
kInode_t *sysOut = NULL;


/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */
int term_create (kSubSystem_t *subsys, kInode_t *frame)
{
  kInode_t *ino;
  kInode_t *ino_out;
  kMemArea_t *area;
  char no[10];
#ifndef _FONT8
  int fontW = 6;
  int fontH = 9;
#else
  int fontW = 8;
  int fontH = 8;
#endif
  int nol = kSYS.ttyAutoInc_++;
  kTerm_t *term;

  snprintf(no, 10, "Tty%d", nol);
  ino = create_inode(no,  kSYS.procIno_, S_IFTTY | 0400, 0);
  assert (ino != NULL);
  create_inode(".in", ino, S_IFIFO | 0400, PAGE_SIZE);
  ino_out = create_inode(".out", ino, S_IFIFO | 0400, 8 * PAGE_SIZE);
  sysOut = ino;

  term = KALLOC(kTerm_t);
  term->txColor_ = 0xffa6a6a6; // 0xff5c5c5c;
  term->bgColor_ = 0xff323232;

  term->pipe_ = fs_create_pipe(ino_out);
  // @Todo -- This is a hugly hack to avoid blocking kwrite!
  ((char*)term->pipe_->mmap_->address_)[0] = 0;
  ((char*)term->pipe_->mmap_->limit_)[-1] = 0;
  term->row_ = 1;
  term->first_ = KALLOC(kLine_t);
  term->first_->txColor_ = 0xffa6a6a6;
  term->first_->bgColor_ = 0xff323232;
  term->last_ = term->first_;
  term->top_ = term->first_;
  inode_open(ino);
  term->ino_ = ino;

  area = area_map_ino(kSYS.mspace_, frame, 0, frame->stat_.length_, 0);
  area->at_ = __AT__;
  term->pixels_ = (void *)area->address_;
  term->pxArea_ = area;

  term->width_ = frame->stat_.block_ / 4;
  term->height_ = frame->stat_.length_ / frame->stat_.block_;


  term->line_ = term->width_;
  term->colMax_ = term->line_ / fontW;
  term->paint = font64_paint;
  term->clear = font64_clean;


  term->max_row_ = (term->height_ - 1) / (fontH + 1);
  term->max_col_ = (term->width_ - 2) / fontW;

  ino->subsys_ = subsys;
  // inon->subsys_ = subsys;

  subsys->term_ = term;
  subsys->out_ = ino;
  // subsys->in_ = inon;

  term->clear(term);

  return __seterrno(0);
}

void term_create_tty (kSubSystem_t *subsys)
{
  kInode_t *ino;
  kInode_t *inon;
  char no[10];
  static int auto_incr = 0;
  int nol = auto_incr++;

  snprintf(no, 10, "Tty%d", nol);
  ino = create_inode(no,  kSYS.procIno_, S_IFIFO | 0400, PAGE_SIZE);
  assert (ino != NULL);

  snprintf(no, 10, ".Tty%d", nol);
  inon = create_inode(no,  kSYS.procIno_, S_IFIFO | 0400, PAGE_SIZE);
  assert (inon != NULL);

  ino->subsys_ = subsys;
  inon->subsys_ = subsys;
  subsys->out_ = ino;
  subsys->in_ = inon;
}

/* ----------------------------------------------------------------------- */
int term_readchar (kTerm_t *term, kLine_t *style, const char **str)
{
  if (**str < 0) {
    /// @todo support UTF-8
    kpanic ("Term is only ASCII\n");
  } else if (**str >= 0x20) {
    return *(*str)++;
  } else if (**str == _ESC || **str == _EOL) {
    return *(*str)++;
  }

  (*str)++;
  return 0x7f;
}


extern uint32_t consoleColor[];
extern uint32_t consoleSecColor[];
extern uint32_t consoleBgColor[];


/* ----------------------------------------------------------------------- */
void term_changecolor(kTerm_t *term, kLine_t *style, int cmd)
{
  if (cmd == 0) {
    style->txColor_ = term->txColor_;
    style->bgColor_ = term->bgColor_;
  } else if (cmd < 30) {
  } else if (cmd < 40) {
    style->txColor_ = consoleColor[cmd - 30];
  } else if (cmd < 50) {
    style->bgColor_ = consoleBgColor[cmd - 40];
  } else if (cmd < 90) {
  } else if (cmd < 100) {
    style->txColor_ = consoleSecColor[cmd - 90];
  } else {
  }
}


/* ----------------------------------------------------------------------- */
void term_readcmd(kTerm_t *term, kLine_t *style, const char **str)
{
  int i;
  int val[5];

  if (**str != '[')
    return;

  i = 0;

  do {
    (*str)++;
    val[i++] = (int)strtoull(*str, (char **)str, 10);

  } while (i < 5 && **str == ';');

  switch (**str) {
  case 'm':
    while (i > 0)
      term_changecolor(term, style, val[--i]);

    break;
  }

  (*str)++;
}


/* ----------------------------------------------------------------------- */
int term_paint (kTerm_t *term, kLine_t *style, int row)
{
  int ch;
  int col = 0;
  const char *base = (char *)term->pipe_->mmap_->address_;
  const char *str = &base[style->offset_]; // String is the offset of the string


  while (*str) {
    ch = term_readchar(term, style, &str);

    if (ch == _EOL) {
      return str - base;
    } else if (ch == _ESC) {
      term_readcmd(term, style, &str);
      continue;
    }

    term->paint (term, style, ch, row, col++);

    if (col >= term->colMax_) /// @todo colMax_
      return str - base; // We return the place of the new line

    if (style->offset_ + col >= (int)term->pipe_->wpen_)
      return 0;
  }

  return 0;
}

// ---------------------------------------------------------------------------
static void term_redraw(kTerm_t *term)
{
  int i;
  int pen;
  kLine_t *start = term->top_;
  term->clear(term);

  for (i = 1; i <= term->max_row_; ++i) {
    kLine_t line = *start;
    pen = term_paint(term, &line, i);

    if (pen == 0)
      break;

    start = start->next_;
  }
}

// ---------------------------------------------------------------------------
static void term_scroll (kTerm_t *term, int count)
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
void term_close (kTerm_t *term)
{
  while (term->first_) {
    kLine_t *l = term->first_->next_;
    kfree (term->first_);
    term->first_ = l;
  }

  area_unmap(kSYS.mspace_, term->pxArea_);
  inode_close(term->ino_);
  kfree(term);
}



/* ----------------------------------------------------------------------- */
void term_write (kTerm_t *term)
{
  kLine_t *newLine;
  int pen; // Where start the next line

  if (term == NULL)
    term = sysLogTty->term_;
  
  for (;;) {

    if (term->row_ > term->max_row_) {
      term_scroll (term, term->row_ - term->max_row_);
    }

    // Paint what seams to be the last line.
    pen = term_paint (term, term->last_, term->row_);
    // pen = term->paint(term, term->last_, term->row_);

    if ((term->flags_ & (TTY_NEW_INPUT | TTY_ON_INPUT)) == TTY_NEW_INPUT)
      term->flags_ |= TTY_READ_ECHO;

    // If it was the last line we're good
    if (pen == 0)
      break;

    // We got a new line here!
    newLine =  KALLOC(kLine_t);
    newLine->offset_ = pen;
    newLine->txColor_ = term->last_->txColor_;
    newLine->bgColor_ = term->last_->bgColor_;
    newLine->flags_ = term->last_->flags_;
    term->last_->next_ = newLine;
    newLine->prev_ = term->last_;
    term->last_ = newLine;

    term->row_++;
  }
}


/* ----------------------------------------------------------------------- */
int kwrite_pipe (const void *m, int lg)
{
  if (lg < 0)
    lg = strlen ((const char*)m);
  lg = fs_pipe_write (sysLogTty->term_->ino_, m, lg);
  term_write(sysLogTty->term_);
  return lg;
}

#define EV_KEYUP 10 // Already in 3, think about header
#define EV_KEYDW 11
#define _BKSP 8

void event_pipe(int type, int value)
{

  kInode_t* ino = search_child2(".in", sysOut);

  switch (type) {
  case EV_KEYDW:
    if (value == _BKSP) {
      //fs_pipe_unget(1);
    } else if (value < 0x80)
      fs_pipe_write(ino, &value, 1);

    break;

  default:
    break;
  }
}

void create_subsys(kInode_t *kbd, kInode_t *screen)
{
  if (screen != NULL) {
    term_create(&frameTty, screen);
    sysLogTty = &frameTty;
  } else {
    term_create_tty(&vgaText);
    sysLogTty = &vgaText;
  }

  if (kbd != NULL) {
    kbd->subsys_ = sysLogTty;
  }
}


void open_subsys(kInode_t *input, kInode_t *output)
{
//   assert(S_ISFIFO(input->stat_.mode_) || S_ISCHR(input->stat_.mode_));
//   assert(S_ISFIFO(output->stat_.mode_) || S_ISCHR(output->stat_.mode_));

//   fs_create_pipe(input);
//   fs_create_pipe(output);

//   sysOut = output;
}

int BMP_sync(kInode_t *);

void clean_subsys()
{
#ifdef _FS_UM
  kInode_t *fb = search_inode ("/dev/Fb0", NULL, 0, NULL);
  BMP_sync(fb);
#endif

  if (sysLogTty->term_)
    term_close (sysLogTty->term_);

  sysLogTty = &vgaText;
  sysOut = NULL;
}


void kwrite(void* buf, int lg) {
  if (sysOut != NULL) {
    kInode_t* ino = search_child2(".out", sysOut);
    fs_pipe_write(ino, buf, lg);
    term_write(sysLogTty->term_);
  } else {

  }
}

