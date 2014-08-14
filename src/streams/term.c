#include <kernel/streams.h>
#include <kernel/inodes.h>

// ===========================================================================

uint64_t ttyFont [] = {
  0x00000000000000, 0x00004004104104, 0x0000000000028a, 0x0000a7ca51e500, // 20-23
  0x0010f30f1453c4, 0x0004d3821072c8, 0x00016255085246, 0x00000000000104, // 24-27
  0x08104104104108, 0x04208208208204, 0x00000284384000, 0x0000411f104000, // 28-2b
  0x00084180000000, 0x0000000e000000, 0x00006180000000, 0x00082104208410, // 2c-2f

  0x0000f24924924f, 0x00004104104106, 0x0000f0413c820f, 0x0000f20838820f, // 30-33 - 0
  0x0000820f249249, 0x0000f2083c104f, 0x0000f24924f04f, 0x0000208210420f, // 34-37 - 4
  0x0000f2493c924f, 0x0000f20f24924f, 0x00006180186000, 0x00084180186000, // 38-3b - 8
  0x00018181198000, 0x00000380380000, 0x00003310303000, 0x0000400410844e, // 3c-3f

  0x1f05d55d45f000, 0x000114517d145f, 0x0001f4517c924f, 0x0000f04104104f, // 40-23
  0x0000f45145144f, 0x0000f0411c104f, 0x000010411c104f, 0x0001f45174104f, // 44-27 - D
  0x000114517d1451, 0x00002082082082, 0x00007208208208, 0x00009143043149, // 48-2b - H
  0x0000f041041041, 0x0001155555555f, 0x0001145145145f, 0x0001f45145145f, // 4c-2f - L

  0x000010417d145f, 0x0041f55145145f, 0x000114517c924f, 0x0000f2083c104f, // 50-23 - P
  0x0000410410411f, 0x0001f451451451, 0x0000410a291451, 0x0001b555555451, // 54-27 - T
  0x0001144a10a451, 0x000041047d1451, 0x0000f04108420f, 0x0c10410410410c, // 58-2b - X
  0x00410208104082, 0x06104104104106, 0x00000000011284, 0x0003f000000000, // 5c-2f

  0x00000000000204, 0x0000f24f20f000, 0x0000f24924f041, 0x0000f04104f000, // 60-23
  0x0000f24924f208, 0x0000f04f24f000, 0x00001041043047, 0x0f20f24924f000, // 64-27 - d
  0x0000924924f041, 0x00002082082002, 0x000c2082082002, 0x00009143149041, // 68-2b - h
  0x00007041041041, 0x0001555555f000, 0x0000924924f000, 0x0000f24924f000, // 6c-2f - l

  0x0104f24924f000, 0x0820f24924f000, 0x0000104124f000, 0x0000f20f04f000, // 70-23 - p
  0x00007041043041, 0x0000f249249000, 0x0000428a451000, 0x0001f555555000, // 74-27 - t
  0x00009246249000, 0x0f20f249249000, 0x0000f04210f000, 0x18104103104118, // 78-2b - x
  0x04104104104104, 0x03104118104103, 0x00000000352000, 0x00000000000000, // 7c-2f
};

static int fontW = 6;
static int fontH = 9;

uint32_t consoleColor[] = {
  0xff181818, 0xffa61010,  0xff10a610, 0xffa6a610, 0xff1010a6, 0xffa610a6, 0xff10a6a6,  0xffa6a6a6, 0xffffffff
};
uint32_t consoleSecColor[] = {
  0xff181818, 0xffa61010,  0xff10a610, 0xffa66810, 0xff1010a6, 0xffa610a6, 0xff10a6a6,  0xff5c5c5c, 0xffffffff
};
uint32_t consoleBgColor[] = {
  0xff000000, 0xff600606,  0xff066006, 0xff606006, 0xff060660, 0xff600660, 0xff066060,  0xff323232, 0xffffffff
};

// ===========================================================================

int getPen (kNTty_t* tty, int row, int col)
{
  return 1+ tty->width_ +
    ((row-1) * (fontH+1)) * tty->width_ +
    col * (fontW);
}

void draw (kNTty_t* tty, int ch, kLine_t* style, int pen, int width)
{
  int i, j;
  uint64_t vl;

  vl = (uint64_t)ttyFont[ch - 0x20];
  for (j=0; j<fontH; ++j) {
    for (i=0; i<fontW; ++i) {
      tty->pixels_ [pen + i] = (vl & 1) ? style->txColor_ : style->bgColor_;
      vl = vl >> 1;
    }
    pen += width;
  }

  if (style->flags_ & 2) {
    for (i=0; i<fontW; ++i) {
      tty->pixels_ [pen + i] = style->txColor_;
    }
  } else {
    for (i=0; i<fontW; ++i) {
      tty->pixels_ [pen + i] = style->bgColor_;
    }
  }
}


char* getCommand (kNTty_t* term, char* str, kLine_t* style)
{
  for (;;) {

    if (*str == '0') {
      str++;
      style->txColor_ = term->txColor_;
      style->bgColor_ = term->bgColor_;
      style->flags_ = 0;

    } else if (*str == '3') {
      str++;
      if (*str >= '0' && *str < '9')
        style->txColor_ = consoleColor[*(str++) - '0'];

    } else if (*str == '4') {
      str++;
      if (*str >= '0' && *str < '9')
        style->bgColor_ = consoleBgColor[*(str++) - '0'];

    } else if (*str == '5') {
      str++;
      if (*str >= '0' && *str < '9')
        style->flags_ = *(str++) - '0';

    } else if (*str == '7') {
      str++;
      uint32_t temp = style->txColor_;
      style->txColor_ = style->bgColor_;
      style->bgColor_ = temp;

    } else if (*str == '9') {
      str++;
      if (*str >= '0' && *str < '9')
        style->txColor_ = consoleSecColor[*(str++) - '0'];

    } else {
      break;
    }


    if (*str == ',') {
      str++;
      continue;
    } else if (*str == 'm') {
      str++;
    }

    break;
  }

  return str;
}

char* getChar (kNTty_t* term, char* str, int* ch, kLine_t* style)
{
  for (;;) {
    if (*str < 0) {

    } else if (*str < 0x20 && *str != '\n' && *str != '\t') {
      if (str[0] == '\r' && str[1] == '\n') {
        str++;
      } else if (str[0] == '\e' && str[1] == '[') {
        str+=2;
        str = getCommand(term, str, style);
        continue;
      } else {
        str++;
        continue;
      }
    }

    *ch = *(str++);
    return str;
  }
}

void clearScreen (kNTty_t* term)
{
  int i, lg = term->width_ * term->height_;
  for (i = 0; i < lg; ++i)
    term->pixels_[i] = term->bgColor_;
}


int paintLine (kNTty_t* term, kLine_t* style, int row)
{
  uint32_t page;

  int ch;
  int pen;
  int col = 0;

  ssize_t poff = ALIGN_DW(style->offset_, PAGE_SIZE);
  kfs_map (term->output_->ino_, poff, &page);
  void* address = kpg_temp_page (&page);
  char* str = ((char*)address) + (style->offset_);

  while (*str) {
    str = getChar (term, str, &ch, style);

    if (ch == '\n') {
      return str - (char*)address + poff;
    }

    pen = getPen (term, row, col);
    draw (term, ch, style, pen, term->width_);

    col++;
    if (col >= term->width_ / fontW) {
      return str - (char*)address + poff;
    }
  }

  return 0;
}

void kstm_redraw_tty (kNTty_t* tty)
{
  int i;
  int pen;
  clearScreen(tty);
  kLine_t* start = tty->top_;
  for (i = 1; i <= tty->maxRow_; ++i) {
    kLine_t line = *start;
    pen = paintLine (tty, &line, i);
    if (pen == 0)
      break;

    start = start->next_;
  }

  // save_bmp (tty->width_, tty->height_, tty->pixels_);
}

void kstm_scroll_tty (kNTty_t* tty, int count)
{
  if (count > 0) {
    while (count != 0) {
      if (tty->top_->next_ == NULL)
        break;
      tty->top_ = tty->top_->next_;
      count--;
      tty->row_--;
    }

  } else {
    while (count != 0) {
      if (tty->top_->prev_ == NULL)
        break;
      tty->top_ = tty->top_->prev_;
      count++;
      tty->row_++;
    }
  }

  kstm_redraw_tty (tty);
}


// ---------------------------------------------------------------------------
kNTty_t tty0;
/**
 * As we need to log early during initialization we need to a temporary tty
 */
void ktty_set_tty0 (uint32_t* base, int width, int height, int depth)
{
  tty0.pixels_ = base;
  tty0.width_ = width;
  tty0.height_ = height;
  // MODE !? TXT / 32B
  tty0.input_ = kstm_create_pipe(O_RDONLY, 4 * _Kb_);
  tty0.output_ = kstm_create_pipe(O_WRONLY, 12 * _Kb_);

  tty0.txColor_ = 0xffa6a6a6;
  tty0.bgColor_ = 0xff323232;
  tty0.first_ = (kLine_t*)kalloc(sizeof(kLine_t));
  tty0.first_->txColor_ = 0xffa6a6a6;
  tty0.first_->bgColor_ = 0xff323232;
  tty0.last_ = tty0.first_;
  tty0.top_ = tty0.first_;
  tty0.row_ = 1;
  tty0.maxRow_ = (height-1) / (fontH + 1);
  tty0.maxCol_ = (width-2) / fontW;
  clearScreen(&tty0);
}


