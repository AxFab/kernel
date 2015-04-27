#include <smkos/kapi.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>

/* ----------------------------------------------------------------------- */
void kwrite_tty(const char *m);
void kwrite_pipe(const char *m);


struct kSubSystem {
  void (*write)(const char *m);
};

struct kSubSystem vgaText = {
  .write = _kwrite_txt
};

struct kSubSystem {
  .write = kwrite_pipe
};


struct kSubSystem *sysLogTty = &vgaText;
kInode_t* sysOut = NULL;


/* ----------------------------------------------------------------------- */
void kwrite_pipe (const char *m) 
{
  int lg = strlen (m);
  fs_pipe_write (sysOut, m, lg, 0);
  
}


void create_subsys(kInode_t* kbd, kInode_t* screen)
{
}


void open_subsys(kInode_t* input, kInode_t* output)
{
  assert(S_ISFIFO(input->stat_.mode_) || S_ISCHR(input->stat_.mode_));
  assert(S_ISFIFO(output->stat_.mode_) || S_ISCHR(output->stat_.mode_));

  fs_create_pipe(input);
  fs_create_pipe(output);

  sysOut = output;
}


/* ----------------------------------------------------------------------- */
void ascii_cmd(const char **m)
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


/* ----------------------------------------------------------------------- */
#ifndef kwrite

void kwrite(const char *m)
{
  sysLogTty->write(m);
}

#endif


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
