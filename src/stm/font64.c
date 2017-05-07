#include <smkos/kernel.h>
#include <smkos/arch.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/term.h>




#define FONT64_PEN(t,r,c,f) \
      ( 1 + (t)->width_ + ( (r - 1) * (f->dispy_ + 1) ) * (t)->width_ + c * f->dispx_ )

#define FONTBMP_PEN(t,r,c,f) \
      ( 1 + (t)->width_ + ( (r - 1) * (f->dispy_ + 1) ) * (t)->width_ + c * f->dispx_ )

// ===========================================================================
//      Font64 - Data
// ===========================================================================

uint64_t ttyFont6x9 [] = {
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
  0x04104104104104, 0x03104118104103, 0x00000000352000, 0x0001f7df7df7df, // 7c-2f
};

uint64_t ttyFont8x8 [] = {
  0x0000000000000000, 0x0000100010101010, 0x0000000000001414, 0x0000143E143E1400,
  0x00103C5038147810, 0x0000609468162906, 0x0000986498040438, 0x0000000000001010,
  0x0000100804040810, 0x0000081020201008, 0x00000054387C3854, 0x000010107C101000,
  0x0008100000000000, 0x000000003C000000, 0x0000100000000000, 0x0000020408102040,

  0x0000182442422418, 0x0000381010101810, 0x00007E041820423C, 0x00003C421820423C,
  0x0000103E12141810, 0x00003C42403E027E, 0x00003C42423E023C, 0x000002040810207E,
  0x00003C42423C423C, 0x00003C407C42423C, 0x0000100000100000, 0x0008100000100000,
  0x0000601806186000, 0x0000007E007E0000, 0x0000061860180600, 0x000008001820221C,

  0x38043A2939223C00, 0x000042423C241818, 0x00001E22221E221E, 0x00001C220101221C,
  0x00001E222222221E, 0x00003E02021E023E, 0x00000202021E023E, 0x00001C223901221C,
  0x00004242427E4242, 0x00007C101010107C, 0x00001C2220202038, 0x000022120E0A1222,
  0x00007E0202020202, 0x0000828292AAC682, 0x00004262524A4642, 0x0000384444444438,

  0x00000202021E221E, 0x0040384444444438, 0x000022120A1E221E, 0x0000384430084438,
  0x00001010101010FE, 0x00003C4242424242, 0x0000182424424281, 0x000082C6AA928282,
  0x0000422418182442, 0x0000101010284482, 0x00007E040810207E, 0x00001C040404041C,
  0x0000402010080402, 0x00001C101010101C, 0x0000000000001408, 0x00007E0000000000,

  0x0000000000001008, 0x00005C627C403C00, 0x00003A46423E0202, 0x0000380404380000,
  0x00005C62427C4040, 0x00003C027E423C00, 0x000008081C081800, 0x1C202C22322C0000,
  0x00002424241C0404, 0x0000101010100010, 0x0E10101010180010, 0x0000340C14240404,
  0x0000180808080808, 0x000042425A660000, 0x000044444C740000, 0x00003C42423C0000,

  0x02023E42463A0000, 0x40407C42625C0000, 0x000004044C340000, 0x00001C2018043800,
  0x00001808083C0800, 0x0000586444440000, 0x0000182442420000, 0x0000665A81810000,
  0x0000661824420000, 0x0608102844420000, 0x00003C08103C0000, 0x000038080C0C0838,
  0x0010101010101010, 0x00001C103030101C, 0x000000324C000000, 0x00007E7E7E7E7E7E,
};


struct bitmap_font font64_8 = {
  .width_ = 8,
  .height_ = 8,
  .dispx_ = 8,
  .dispy_ = 8,
  .glyph_size_ = 8,
  .glyph_ = (uint8_t*)ttyFont8x8,
};

struct bitmap_font font64_6_9 = {
  .width_ = 6,
  .height_ = 9,
  .dispx_ = 6,
  .dispy_ = 9,
  .glyph_size_ = 8,
  .glyph_ = (uint8_t*)ttyFont6x9,
};


#ifndef _FONT8
const int fontW = 6;
const int fontH = 9;
uint64_t *ttyFont = ttyFont6x9;
#else
const int fontW = 8;
const int fontH = 8;
uint64_t *ttyFont = ttyFont8x8;
#endif

uint32_t consoleColor[] = {
  0xff181818, 0xffa61010, 0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6, 0xffa6a6a6,
  0xffffffff
};
uint32_t consoleSecColor[] = {
  0xff323232, 0xffd01010, 0xff10d010, 0xffd0d010,
  0xff1060d0, 0xffd010d0, 0xff10d0d0, 0xffd0d0d0,
  0xffffffff
};
uint32_t consoleBgColor[] = {
  0xff181818, 0xffa61010, 0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6, 0xffa6a6a6,
  0xffffffff
};

// ===========================================================================
//      Font64 - Methods
// ===========================================================================



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
  * @return           The value is the offset of the end of line if marked
  */
void font64_paint (kTerm_t *term, kLine_t *style, int ch, int row, int col)
{
  int pen = FONT64_PEN(term, row, col, fontbmp);
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


// ===========================================================================
//      Bitmap font - Methods
// ===========================================================================


// ---------------------------------------------------------------------------

static void fontbmp_draw (kTerm_t *term, int ch, kLine_t *style, int pen, int width)
{
  int i, j, l = 0;
  const uint8_t *glyph = &(fontbmp->glyph_[(ch - 32) * fontbmp->glyph_size_]);

  for (j = 0; j < fontbmp->height_; ++j) {
    for (i = 0; i < fontbmp->width_; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = ((*glyph) & (1 << l)) ? style->txColor_ : style->bgColor_;
      if (++l >= 8) {
        glyph++;
        l = 0;
      }
    }
    for (; i < fontbmp->dispx_; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = style->bgColor_;
    }

    pen += width;
  }
  for (; j < fontbmp->dispy_; ++j) {
    for (i = 0; i < fontbmp->dispx_; ++i) {
      ((uint32_t *)term->pixels_)[pen + i] = style->bgColor_;
    }
    pen += width;
  }

  uint32_t underline = style->flags_ & 2 ? style->txColor_ : style->bgColor_;
  for (i = 0; i < fontbmp->dispx_; ++i) {
    ((uint32_t *)term->pixels_)[pen + i] = underline;
  }
}

// ===========================================================================
//      Bitmap font - Interface
// ===========================================================================

// ---------------------------------------------------------------------------
/** Paint a text line on the frame buffer used by this terminal
  * @return           The value is the offset of the end of line if marked
  */
void fontbmp_paint (kTerm_t *term, kLine_t *style, int ch, int row, int col)
{
  int pen = FONTBMP_PEN(term, row, col, fontbmp);
  fontbmp_draw (term, ch, style, pen, term->width_);

}


// ---------------------------------------------------------------------------
/**
  */
void fontbmp_clean(kTerm_t *term)
{
  int i;
  int lg = term->width_ * term->height_;

  for (i = 0; i < lg; ++i)
    ((uint32_t *)term->pixels_)[i] = term->bgColor_;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

