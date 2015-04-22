#include <smkos/kernel.h>
#include <smkos/core.h>

struct kTty {

  void (*write)(const char *m);
};



/* ----------------------------------------------------------------------- */
void _kwrite_font64(const char *m)
{
}

/* ----------------------------------------------------------------------- */
int ktty(kInode_t* ino)
{
  int idx;
  kMemArea_t* area;
  unsigned int* pixels;
  struct kTty* tty;

  area = area_map_ino(kSYS.mspace_, ino, 0, ino->stat_.length_, 0);
  pixels = (unsigned int*)area->address_;
  for (idx = 0; idx < (ino->stat_.length_ / 4); idx++) {
    pixels[idx] = 0x181818; 
  }

  tty = KALLOC(struct kTty);
  tty->write = _kwrite_font64;
}


/* ----------------------------------------------------------------------- */

void ascii_cmd(const char **m)
{
  int idx = 0;
  char sign;
  char *mL;
  int values[5];

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

static uint16_t* txtBuffer = (uint16_t*)0xB8000;
static int txtIdx = 0;

void _kwrite_txt(const char *m)
{
  
  for (; *m; ++m) {
    if (*m < 0x20) {
      if (*m == '\n') 
        txtIdx += 80 - (txtIdx % 80);
      else if (*m == '\t')
        txtIdx += 4 - (txtIdx % 4);
      else if (*m == '\e' && m[1] == '[') {
        ++m;
        ascii_cmd(&m);
      }
      continue;
    }

    txtBuffer[txtIdx] = (*m & 0xff) | 0x700;
    txtIdx++;
  }
}

struct kTty vgaText = {
  .write = _kwrite_txt
};
struct kTty *sysLogTty = &vgaText;

/* ----------------------------------------------------------------------- */
#ifndef kwrite

void kwrite(const char *m)
{
  sysLogTty->write(m);
}

#endif


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
