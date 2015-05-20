#include <smkos/kernel.h>
#include <smkos/arch.h>
#include <smkos/kstruct/user.h>

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
void kwrite_tty(const char *m)
{

  for (; *m; ++m) {
    if (*m < 0x20) {
      if (*m == '\n')
        txtOutIdx += 80 - (txtOutIdx % 80);
      else if (*m == '\r')
        txtOutIdx -= (txtOutIdx % 80);
      else if (*m == '\t')
        txtOutIdx += 4 - (txtOutIdx % 4);
      else if (*m == '\e' && m[1] == '[') {
        ++m;
        ascii_cmd(&m);
      }

      continue;
    }

    txtOutBuffer[txtOutIdx++] = (*m & 0xff) | 0x700;

    if (txtOutIdx > 1840) {
      memcpy((void *)0xB8000, (void *)(0xB8000 + 80 * 2), 1840 * 2);
      txtOutIdx -= 80;
      memset((void *)(0xB8000 + 1840 * 2), 0, 80 * 2);
    }
  }
}


/* ----------------------------------------------------------------------- */
void event_tty(int type, int value)
{
  if (type == 10) {
    if ((value >= 0x20 && value < 0x80) && txtInIdx < 80) {
      txtInBuffer[txtInIdx++] = (value & 0xff) | 0x700;
      show_cursor_vga_text(24, txtInIdx);
    } else if (value == '\n') {
      memset (txtInBuffer, 0, 80 * 2);
      txtInIdx = 0;
    } else if (value == /*KEY_BACKSPACE*/8 && txtInIdx > 0) {
      txtInBuffer[--txtInIdx] = 0x700;
      show_cursor_vga_text(24, txtInIdx);
    }
  }
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
