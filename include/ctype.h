#pragma once

struct charloc {
  char lower_, upper_;
  short flags_;
};

typedef struct charloc locale_t[256];

#define CTYPE_CNTRL 0x001
#define CTYPE_BLANK 0x002
#define CTYPE_SPACE 0x004
#define CTYPE_GRAPH 0x008
#define CTYPE_PUNCT 0x010
#define CTYPE_DIGIT 0x020
#define CTYPE_XDIGT 0x040
#define CTYPE_ALPHA 0x080
#define CTYPE_LOWER 0x100
#define CTYPE_UPPER 0x200

#define _getLocale()  (stdlocal)
/* ----------------------------------------------------------------------- */
extern locale_t stdlocal;

/* ----------------------------------------------------------------------- */
int isctype_(int c, int flg, locale_t local);
int isascii(int c);

/* ----------------------------------------------------------------------- */
/** checks for an alphanumeric character */
#define isalnum(c)  isctype_(c, CTYPE_ALPHA | CTYPE_DIGIT, _getLocale())
/** checks for an alphabetic character */
#define isalpha(c)  isctype_(c, CTYPE_ALPHA, _getLocale())
/** checks for a blank character */
#define isblank(c)  isctype_(c, CTYPE_BLANK, _getLocale())
/** checks for a control character */
#define iscntrl(c)  isctype_(c, CTYPE_CNTRL, _getLocale())
/** checks for a digit (0 through 9) */
#define isdigit(c)  isctype_(c, CTYPE_DIGIT, _getLocale())
/** checks for a xdigit (0 through F) */
#define isxdigit(c)  isctype_(c, CTYPE_XDIGT, _getLocale())
/** checks for any printable character except space */
#define isgraph(c)  isctype_(c, CTYPE_GRAPH, _getLocale())
/** checks for a lowercase character */
#define islower(c)  isctype_(c, CTYPE_LOWER, _getLocale())
/** checks for any printable character which is not a space or an 
  * alphanumeric character */
#define ispunct(c)  isctype_(c, CTYPE_PUNCT, _getLocale())
/** checks for white-space characters */
#define isspace(c)  isctype_(c, CTYPE_SPACE, _getLocale())
/** checks for an uppercase letter */
#define isupper(c)  isctype_(c, CTYPE_UPPER, _getLocale())
/** checks for hexadecimal digits */
#define isxdigit(c)  isctype_(c, CTYPE_XDIGT, _getLocale())

/** checks for any printable character including space */
int isprint (int c);
/** convert to lowercase */
int tolower (int c);
/** convert to uppercase */
int toupper (int c);

/* ----------------------------------------------------------------------- */
/** checks for an alphanumeric character */
#define isalnum_l(c)  isctype_(c, CTYPE_ALPHA | CTYPE_DIGIT, l)
/** checks for an alphabetic character */
#define isalpha_l(c,l)  isctype_(c, CTYPE_ALPHA, l)
/** checks for a blank character */
#define isblank_l(c,l)  isctype_(c, CTYPE_BLANK, l)
/** checks for a control character */
#define iscntrl_l(c,l)  isctype_(c, CTYPE_CNTRL, l)
/** checks for a digit (0 through 9) */
#define isdigit_l(c,l)  isctype_(c, CTYPE_DIGIT, l)
/** checks for a xdigit (0 through F) */
#define isxdigit_l(c,l)  isctype_(c, CTYPE_XDIGT, l)
/** checks for any printable character except space */
#define isgraph_l(c,l)  isctype_(c, CTYPE_GRAPH, l)
/** checks for a lowercase character */
#define islower_l(c,l)  isctype_(c, CTYPE_LOWER, l)
/** checks for any printable character which is not a space or an 
  * alphanumeric character */
#define ispunct_l(c,l)  isctype_(c, CTYPE_PUNCT, l)
/** checks for white-space characters */
#define isspace_l(c,l)  isctype_(c, CTYPE_SPACE, l)
/** checks for an uppercase letter */
#define isupper_l(c,l)  isctype_(c, CTYPE_UPPER, l)
/** checks for hexadecimal digits */
#define isxdigit_l(c,l)  isctype_(c, CTYPE_XDIGT, l)

/** checks for any printable character including space */
int isprint_l (int c, locale_t locale);
/** convert to lowercase */
int tolower_l (int c, locale_t locale);
/** convert to uppercase */
int toupper_l (int c, locale_t locale);

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

