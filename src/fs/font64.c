#include <smkos/kernel.h>
#include <smkos/arch.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/term.h>




#define FONT64_PEN(t,r,c) \
      ( 1 + (t)->width_ + ( (r - 1) * (fontH + 1) ) * (t)->width_ + c * fontW )

// ===========================================================================
//      Font64 - Data
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

const int fontW = 6;
const int fontH = 9;


uint32_t consoleColor[] = {
  0xff181818, 0xffa61010,  0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6,  0xffa6a6a6,
  0xffffffff
};
uint32_t consoleSecColor[] = {
  0xff323232, 0xffd01010,  0xff10d010, 0xffd0d010,
  0xff1060d0, 0xffd010d0, 0xff10d0d0,  0xffd0d0d0,
  0xffffffff
};
uint32_t consoleBgColor[] = {
  0xff181818, 0xffa61010,  0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6,  0xffa6a6a6,
  0xffffffff
};

// ===========================================================================
//      Font64 - Methods
// ===========================================================================

// ---------------------------------------------------------------------------
/** Read a text command '\e[xxa'
  */
static const char *font64_cmd(kTerm_t *term, const char *str, kLine_t *style)
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

// ---------------------------------------------------------------------------
/** Read the next character of the line
  * @bug, what if a command is at the end of the buffer (no '\n')
  */
static const char *font64_getc(kTerm_t *term, const char *str, int *ch, kLine_t *style)
{
  for (;;) {
    if (*str < 0) {

    } else if (*str < 0x20 && *str != '\n' && *str != '\t') {
      if (str[0] == '\r' && str[1] == '\n') {
        str++;
      } else if (str[0] == '\e' && str[1] == '[') {
        str += 2;
        str = font64_cmd(term, str, style);
        continue;
      } else {
        // @todo handle UTF-8 characters
        str++;
        continue;
      }
    }

    *ch = *(str++);
    return str;
  }
}


// ---------------------------------------------------------------------------

static void font64_draw (kTerm_t *term, int ch, kLine_t *style, int pen, int width)
{
  int i, j;
  uint64_t vl;

  vl = (uint64_t)ttyFont[ch - 0x20];

  for (j = 0; j < fontH; ++j) {
    for (i = 0; i < fontW; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = (vl & 1) ? style->txColor_ : style->bgColor_;
      vl = vl >> 1;
    }

    pen += width;
  }

  if (style->flags_ & 2) {
    for (i = 0; i < fontW; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = style->txColor_;
    }
  } else {
    for (i = 0; i < fontW; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = style->bgColor_;
    }
  }
}

// ===========================================================================
//      Font64 - Interface
// ===========================================================================

// ---------------------------------------------------------------------------
/** Paint a text line on the frame buffer used by this terminal
  * @param[in] fb     Frame buffer of the terminal
  * @return           The value is the offset of the end of line if marked
  */
void font64_paint (kTerm_t *term, kLine_t *style, int ch, int row, int col)
{
  int pen = FONT64_PEN(term, row, col);
  font64_draw (term, ch, style, pen, term->width_);

}


// ---------------------------------------------------------------------------
/**
  */
void font64_clean(kTerm_t *term)
{
  int i;
  int lg = term->width_ * term->height_;

  for (i = 0; i < lg; ++i)
    ((uint32_t *)term->pixels_)[i] = term->bgColor_;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

