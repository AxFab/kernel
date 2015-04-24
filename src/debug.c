#include <smkos/kapi.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
// #include <smkos/core.h>
// #include <smkos/_spec.h>
// #include <smkos/_types.h>


// ----------------------------------------------------------------------------
typedef struct kSymbol kSymbol_t;
struct kSymbol {
  const char *name_;
  uintptr_t   address_;
  kSymbol_t  *next_;
};

kSymbol_t *first;
kSymbol_t *last;

// ----------------------------------------------------------------------------
void ksymreg (uintptr_t ptr, const char *sym)
{

  if (first == NULL) {
    first = KALLOC (kSymbol_t);
    last = first;
  } else {
    last->next_ =  KALLOC (kSymbol_t);
    last = last->next_;
  }

  // kprintf ("[%6x] <%s>\n", (uint32_t)ptr, sym);
  last->name_ = strdup(sym);
  last->address_ = ptr;
}

// ----------------------------------------------------------------------------
const char *ksymbol (void *address)
{
  kSymbol_t *iter = first;

  if (first == NULL)
    return "__unamed__";

  if ((uintptr_t)address < first->address_)
    return "<<<<<";

  while (iter->next_) {
    if (iter->next_->address_ > (uintptr_t)address)
      return iter->name_;

    iter = iter->next_;
  }

  return ">>>>>";
}

// ----------------------------------------------------------------------------
/**
    Print the stack trace of the current frame
    FIXME: We can improve readablility by parsing the .map file
 */
void kstacktrace(size_t MaxFrames)
{
  size_t frame;
  size_t *ebp = &MaxFrames - 2;
  size_t *arguments;
  kprintf("Stack trace: [%x]\n", (size_t)ebp);

  for (frame = 0; frame < MaxFrames; ++frame) {
    size_t eip = ebp[1];

    if (eip == 0)
      // No caller on stack
      break;

    // Unwind to previous stack frame
    ebp = (size_t *)(ebp[0]);
    arguments = &ebp[2];
    kprintf("  0x%x - %s ()         [args: %x] \n", eip, ksymbol((void *)eip), (size_t)arguments);
  }
}

// ----------------------------------------------------------------------------
/**
    Display hexadeciaml data of a memory area.
 */
void kdump (void *ptr, size_t lg)
{
  int i;

  while (lg > 0) {
    kprintf ("0x%8x  ", (unsigned int)ptr);

    for (i = 0; i < 16; ++i)
      kprintf (" %02x", ((uint8_t *)ptr)[i]);

    kprintf ("  ");

    for (i = 0; i < 16; ++i) {
      if (((uint8_t *)ptr)[i] < 0x20) kprintf (".");

      else if (((uint8_t *)ptr)[i] > 0x7f) kprintf (".");

      else kprintf ("%c", ((uint8_t *)ptr)[i]);
    }

    kprintf ("\n");
    lg -= 16;
    ptr = &((uint8_t *)ptr)[16];
  }

  kprintf ("\n");
}



/* ======================================================================= */
#define SC_TEXT   1
#define SC_DATA   2
#define SC_BSS    3

void ksymbols_load (kInode_t* ino)
{
  int i;
  int j;
  int state = 0;
  int lg = ino->stat_.length_;
  uintptr_t ptr;
  kMemArea_t* area;
  char *tmp;
  char *str = kalloc (512);
  char *sym = kalloc (512);
  char *add = kalloc (20);

  return;
  area = area_map_ino(kSYS.mspace_, ino, 0, ino->stat_.length_, 0);
  tmp = (char *)area->address_;

  while (i < lg) {

    j = 0;
    while (tmp[i + j] != '\n' && (i + j) < lg) {
      str[j] = tmp[i + j];
      ++j;
    }

    str[++j] = '\0';
    i += j;

    if (str[0] != ' ') state = 0;

    if (state == 0) {
      if (strncmp (str, ".text", 5) == 0)         state = SC_TEXT;
      else if (strncmp (str, ".data", 5) == 0)    state = SC_DATA;
      else if (strncmp (str, ".bss", 4) == 0)     state = SC_BSS;

    } else {
      if (str[1] != ' ')                          continue;
      strncpy (add, &str[16], 18);
      strcpy (sym, &str[50]);
      sym[strlen(sym)-1] = '\0';
      add[19] = '\0';
      ptr = strtoull (add, NULL, 0);

      ksymreg (ptr, sym);
    }
  }

  kprintf("Symbol loaded\n");

  kfree (str);
  kfree (sym);
  kfree (add);
  area_unmap(kSYS.mspace_, area);
  // kfs_release (ino);
}

