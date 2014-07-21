#ifndef _TTY_H__
#define _TTY_H__

#include <kcore.h>

enum kTty_Style {
	White = 0x0f00,
	Regular = 0x0700,
	Matrix = 0x0a00,
} kTty_Style_e;


typedef struct kTty kTty_t;

struct kTty 
{
  uint32_t  _color;
  uint32_t  _bkground;
  int       _cursorX;
  int       _cursorY;
  int       _mode; // TODO txtmode
  int       _column;
  int       _row;
  int       _width;
  int       _height;
  int       _depth;
  uint32_t* _ptr;
  uint32_t  _length;
};

extern kTty_t screen;


void kTty_MoveCursor (int pos);
void kTty_ShowCursor (void);
void kTty_HideCursor (void);
int kTty_Initialize (void);
void kTty_Write (const char *str);
void kTty_Putc (char c);
void kTty_Eol ();
void kTty_Scroll ();


void kTty_KeyPress (int c);
void kTty_KeyUp (int c);


void kTty_HexChar (unsigned int value, int size);
void kTty_HexDump (unsigned char* ptr, int length);

void kTty_Update ();
void kTty_NewTerminal (uintptr_t base, size_t limit);

#endif /* _TTY_H__ */
