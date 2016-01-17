#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/arch.h>
#include <smkos/kstruct/user.h>
#include <smkos/kstruct/term.h>


#undef INT_MAX
#define INT_MAX ((int)2147483647)

#undef INT_MIN
#define INT_MIN ((int)-INT_MAX - 1)


/* ----------------------------------------------------------------------- */
static uint16_t *txtOutBuffer = (uint16_t *)0xB8000;
static int txtOutIdx = 0;
static uint16_t *txtInBuffer = (uint16_t *)0xB8F00;
static int txtInIdx = 0;


/* ----------------------------------------------------------------------- */
static void ascii_cmd(const char **m)
{
  // int idx = 0;
  // char sign;
  // char *mL;
  // int values[5];

  for (;;) {

    while (**m != 'm')
      (*m)++;

    // values[idx] = _strtox(m, &mL, 10, &sign);
    // if (**m == *mL)
    //   return;

    // *m = mL;
    return;
    /*
    switch (**m) {
      case ';':
        (*m)++;
        if (++idx >= 5)
          return;
        continue;

      case 'm':
        (*m)++;
        // Change color
        break;

      default:
        return;
    }
    */
  }
}

/* ----------------------------------------------------------------------- */
static void show_cursor_vga_text (int row, int col)
{
  /// @todo -- This has nothing to do here !!
  uint16_t pos = row * 80 + col;
  outb(0x3d4, 0x0f);
  outb(0x3d5, (uint8_t)pos);
  outb(0x3d4, 0x0e);
  outb(0x3d5, (uint8_t)(pos >> 8));
}


/* ----------------------------------------------------------------------- */
int kwrite_tty(const void *m, int lg)
{
  const char* s = (const char*)m;
  if (lg < 0)
    lg = INT_MAX;
  for (; *s && lg-- > 0; ++s) {
    if (*s < 0x20) {
      if (*s == '\n')
        txtOutIdx += 80 - (txtOutIdx % 80);
      else if (*s == '\r')
        txtOutIdx -= (txtOutIdx % 80);
      else if (*s == '\t')
        txtOutIdx += 4 - (txtOutIdx % 4);
      else if (*s == '\e' && s[1] == '[') {
        ++s;
        ascii_cmd(&s);
      }

      continue;
    }

    if (txtOutIdx > 1840) {
      memcpy((void *)0xB8000, (void *)(0xB8000 + 80 * 2), 1840 * 2);
      txtOutIdx -= 80;
      memset((void *)(0xB8000 + 1840 * 2), 0, 80 * 2);
    }

    txtOutBuffer[txtOutIdx++] = (*s & 0xff) | 0x700;
  }

  return (int)(s - (const char*)m);
}


/* ----------------------------------------------------------------------- */

extern kSubSystem_t *sysLogTty;
#define EV_KEYUP 10
#define EV_KEYDW 11
#define _BKSP 0x08
#define _ESC 0x1B
#define _EOL '\n'


void event_tty(int type, int value)
{
  kInode_t *ino = sysLogTty->in_;

  switch (type) {
  case EV_KEYDW:
    if (value == _BKSP && txtInIdx > 0) {
      //fs_pipe_unget(1);
      txtInBuffer[--txtInIdx] = 0x700;
      show_cursor_vga_text(24, txtInIdx);

    } else if (value == _EOL) {
      fs_pipe_write(ino, &value, 1);
      memset (txtInBuffer, 0, 80 * 2);
      txtInIdx = 0;

    } else if (value < 0x80) {
      fs_pipe_write(ino, &value, 1);
      if ((value >= 0x20 && value < 0x80) && txtInIdx < 80) {
        txtInBuffer[txtInIdx++] = (value & 0xff) | 0x700;
        show_cursor_vga_text(24, txtInIdx);
      }
    }

    break;

  default:
    break;
  }

  // if (type == 10) {
  //   if ((value >= 0x20 && value < 0x80) && txtInIdx < 80) {
  //     txtInBuffer[txtInIdx++] = (value & 0xff) | 0x700;
  //     show_cursor_vga_text(24, txtInIdx);
  //   } else if (value == '\n') {
  //     memset (txtInBuffer, 0, 80 * 2);
  //     txtInIdx = 0;
  //   } else if (value == /*KEY_BACKSPACE*/8 && txtInIdx > 0) {
  //     txtInBuffer[--txtInIdx] = 0x700;
  //     show_cursor_vga_text(24, txtInIdx);
  //   }
  // }
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
