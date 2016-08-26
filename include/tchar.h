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
 */
#ifndef _TCHAR_H
#define _TCHAR_H 1


#if defined _MBCS
# include <mbstring.h>
# define TCHAR unsigned char
# define _TCS_SFX(n)  mbs ## n
#elif defined _UNICODE
# include <wchar.h>
# define TCHAR wchar_t
# define _TCS_SFX(n)  wcs ## n
#else 
# include <string.h>
# define TCHAR char
# define _TCS_SFX(n)  str ## n
#endif


/* Appends src to dest. */
#define tcscat _TCS_SFX(cat)
/* Finds c in str. */
#define tcschr _TCS_SFX(chr)
/* Compares one string to another. */
#define tcscmp _TCS_SFX(cmp)
/* Copies string src to dest. */
#define tcscpy _TCS_SFX(cpy)
/* Scans a string. */
#define tcscspn _TCS_SFX(cspn)
/* Performs case-insensitive string comparison. */
#define tcsicmp _TCS_SFX(icmp)
/* Calculates length of a string. */
#define tcslen _TCS_SFX(len)
/* Appends at most maxlen characters of src to dest. */
#define tcsncat _TCS_SFX(ncat)
/* Compares at most maxlen characters of one string to another. */
#define tcsncmp _TCS_SFX(ncmp)
/* Copies at most maxlen characters of src to dest. */
#define tcsncpy _TCS_SFX(ncpy)
/* Scans one string for the first occurrence of any character that's in a second string. */
#define tcspbrk _TCS_SFX(pbrk)
/* Finds the last occurrence of c in str. */
#define tcsrchr _TCS_SFX(rchr)
/* Scans a string for a segment that is a subset of a set of characters. */
#define tcsspn _TCS_SFX(spn)
/* Finds the first occurrence of a substring in another string. */
#define tcsstr _TCS_SFX(str)
/* Scans s1 for the first token not contained in s2. */
#define tcstok _TCS_SFX(tok)
#define tcstok_r _TCS_SFX(tok_r)
/* duplicate a string  */
#define tcsdup _TCS_SFX(dup)
/* duplicate a string at most maxlen characters */
#define tcsndup _TCS_SFX(ndup)
/* Convert a string to lowercase. */
#define tcslwr _TCS_SFX(lwr)
/* Convert a string to uppercase. */
#define tcsupr _TCS_SFX(upr)
/* Compare characters of two strings without regard to case. */
#define tcsnicmp _TCS_SFX(nicmp)
/* Reverse characters of a string. */
#define tcsrev _TCS_SFX(rev)
/* Set characters of a string to a character. */
#define tcsset _TCS_SFX(set)
/* Initialize characters of a string to a given format. */
#define tcsnset _TCS_SFX(nset)

/* Compare strings using locale-specific information. */
#define tcscoll _TCS_SFX(coll)
#define tcsicoll _TCS_SFX(icoll)
#define tcsncoll _TCS_SFX(ncoll)
#define tcsnicoll _TCS_SFX(nicoll)

/* String transformation */ 
#define tcsxfrm _TCS_SFX(xfrm)


ssize_t enc2tsc(int enc, TCHAR *dest, void *src, size_t lg);
ssize_t tsc2enc(int enc, void *dest, TCHAR *src, size_t lg);
ssize_t enc2enc(int enc1, void *dest, int enc2, void *src, size_t lg);

#define ENC_UTF8 0
#define ENC_UTF16LE 1
#define ENC_UTF16BE 2
#define ENC_UTF32 3
#define ENC_ASCII 4
#define ENC_CP1252 5
#define ENC_ISO8859_1 6
#define ENC_ISO8859_15 7



#endif  /* _TCHAR_H */
