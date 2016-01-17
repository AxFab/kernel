#include <ctype.h>


/* ----------------------------------------------------------------------- */
int isctype_(int c, int flg, locale_t local)
{
  if (c < 0 || c >= 128)
    return 0;

  return local[c].flags_ & flg;
}


/* ----------------------------------------------------------------------- */
/** checks for any printable character including space */
int isprint (int c)
{
  if (c < 0 || c >= 128)
    return 0;

  return (_getLocale()[c].flags_ & CTYPE_GRAPH ) || ( c == ' ' );
}

/** convert to lowercase */
int tolower (int c)
{
  if (c < 0 || c >= 128)
    return c;

  return _getLocale()[c].lower_;
}

/** convert to uppercase */
int toupper (int c)
{
  if (c < 0 || c >= 128)
    return c;

  return _getLocale()[c].upper_;
}

/* ----------------------------------------------------------------------- */
/** checks for any printable character including space */
int isprint_l (int c, locale_t locale)
{
  if (c < 0 || c >= 128) return 0;

  return (locale[c].flags_ & CTYPE_GRAPH ) || ( c == ' ' );
}

/** convert to lowercase */
int tolower_l (int c, locale_t locale)
{
  if (c < 0 || c >= 128) return c;

  return locale[c].lower_;
}

/** convert to uppercase */
int toupper_l (int c, locale_t locale)
{
  if (c < 0 || c >= 128) return c;

  return locale[c].upper_;
}

/* ----------------------------------------------------------------------- */
int isascii(int c)
{
  return (c & (~0x7f));
}


/* ----------------------------------------------------------------------- */
locale_t stdlocal = {
  { 0x00, 0x00, CTYPE_CNTRL },
  { 0x01, 0x01, CTYPE_CNTRL },
  { 0x02, 0x02, CTYPE_CNTRL },
  { 0x03, 0x03, CTYPE_CNTRL },
  { 0x04, 0x04, CTYPE_CNTRL },
  { 0x05, 0x05, CTYPE_CNTRL },
  { 0x06, 0x06, CTYPE_CNTRL },
  { 0x07, 0x07, CTYPE_CNTRL },
  { 0x08, 0x08, CTYPE_CNTRL },
  { 0x09, 0x09, CTYPE_CNTRL | CTYPE_BLANK | CTYPE_SPACE },
  { 0x0a, 0x0a, CTYPE_CNTRL | CTYPE_SPACE },
  { 0x0b, 0x0b, CTYPE_CNTRL | CTYPE_SPACE },
  { 0x0c, 0x0c, CTYPE_CNTRL | CTYPE_SPACE },
  { 0x0d, 0x0d, CTYPE_CNTRL | CTYPE_SPACE },
  { 0x0e, 0x0e, CTYPE_CNTRL },
  { 0x0f, 0x0f, CTYPE_CNTRL },
  { 0x10, 0x10, CTYPE_CNTRL },
  { 0x11, 0x11, CTYPE_CNTRL },
  { 0x12, 0x12, CTYPE_CNTRL },
  { 0x13, 0x13, CTYPE_CNTRL },
  { 0x14, 0x14, CTYPE_CNTRL },
  { 0x15, 0x15, CTYPE_CNTRL },
  { 0x16, 0x16, CTYPE_CNTRL },
  { 0x17, 0x17, CTYPE_CNTRL },
  { 0x18, 0x18, CTYPE_CNTRL },
  { 0x19, 0x19, CTYPE_CNTRL },
  { 0x1a, 0x1a, CTYPE_CNTRL },
  { 0x1b, 0x1b, CTYPE_CNTRL },
  { 0x1c, 0x1c, CTYPE_CNTRL },
  { 0x1d, 0x1d, CTYPE_CNTRL },
  { 0x1e, 0x1e, CTYPE_CNTRL },
  { 0x1f, 0x1f, CTYPE_CNTRL },
  { 0x20, 0x20, CTYPE_BLANK | CTYPE_SPACE },
  { 0x21, 0x21, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x22, 0x22, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x23, 0x23, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x24, 0x24, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x25, 0x25, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x26, 0x26, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x27, 0x27, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x28, 0x28, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x29, 0x29, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2a, 0x2a, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2b, 0x2b, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2c, 0x2c, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2d, 0x2d, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2e, 0x2e, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x2f, 0x2f, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x30, 0x30, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x31, 0x31, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x32, 0x32, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x33, 0x33, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x34, 0x34, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x35, 0x35, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x36, 0x36, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x37, 0x37, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x38, 0x38, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x39, 0x39, CTYPE_GRAPH | CTYPE_DIGIT | CTYPE_XDIGT },
  { 0x3a, 0x3a, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x3b, 0x3b, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x3c, 0x3c, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x3d, 0x3d, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x3e, 0x3e, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x3f, 0x3f, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x40, 0x40, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x61, 0x41, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x62, 0x42, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x63, 0x43, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x64, 0x44, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x65, 0x45, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x66, 0x46, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER | CTYPE_XDIGT },
  { 0x67, 0x47, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x68, 0x48, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x69, 0x49, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6a, 0x4a, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6b, 0x4b, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6c, 0x4c, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6d, 0x4d, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6e, 0x4e, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x6f, 0x4f, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x70, 0x50, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x71, 0x51, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x72, 0x52, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x73, 0x53, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x74, 0x54, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x75, 0x55, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x76, 0x56, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x77, 0x57, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x78, 0x58, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x79, 0x59, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x7a, 0x5a, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_UPPER },
  { 0x5b, 0x5b, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x5c, 0x5c, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x5d, 0x5d, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x5e, 0x5e, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x5f, 0x5f, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x60, 0x60, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x61, 0x41, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x62, 0x42, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x63, 0x43, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x64, 0x44, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x65, 0x45, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x66, 0x46, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER | CTYPE_XDIGT },
  { 0x67, 0x47, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x68, 0x48, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x69, 0x49, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6a, 0x4a, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6b, 0x4b, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6c, 0x4c, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6d, 0x4d, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6e, 0x4e, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x6f, 0x4f, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x70, 0x50, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x71, 0x51, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x72, 0x52, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x73, 0x53, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x74, 0x54, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x75, 0x55, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x76, 0x56, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x77, 0x57, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x78, 0x58, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x79, 0x59, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x7a, 0x5a, CTYPE_ALPHA | CTYPE_GRAPH | CTYPE_LOWER },
  { 0x7b, 0x7b, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x7c, 0x7c, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x7d, 0x7d, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x7e, 0x7e, CTYPE_GRAPH | CTYPE_PUNCT },
  { 0x7f, 0x7f, CTYPE_CNTRL }
};


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
