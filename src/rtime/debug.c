#include <smkos/kernel.h>
#include <smkos/_spec.h>
#include <smkos/_types.h>


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


