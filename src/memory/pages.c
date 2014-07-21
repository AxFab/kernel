#include <memory.h>

/** 
  Table   xxxx----HDA--UWP b

  P: Is the page present in memory
  W: Is the page writable
  U: Is the page accessible in user space
  A: Read operation happends
  D: Write operation happends
  H: Huge page

 */
#define PG_KERNEL_ONLY    3
#define PG_USER_RDWR      7
#define PG_USER_RDONLY    5

#define TABLE_DIR_THR ((uint32_t *)0xfffff000)
#define TABLE_DIR_PRC ((uint32_t *)0xffffe000)
#define TABLE_DIR_KRN ((uint32_t *)0xffffd000)
#define TABLE_DIR_WIN ((uint32_t *)0xffffc000)

#define TABLE_PAGE_TH(s) ((uint32_t *)(0xffc00000 | ((s) << 12)))


// ---------------------------------------------------------------------------
void kPg_DumpTable (uint32_t *table)
{
  int i;
  for (i=0; i<1024; ++i) {
    if ((i % 128) == 0)
      kTty_Putc ('\n');
    if (table[i] == 0)
      kTty_Putc ('.');
    else if ((table[i] & 0x7) == 7)
      kTty_Putc ('U');
    else if ((table[i] & 0x7) == 3)
      kTty_Putc ('K');
    else if ((table[i] & 0x7) == 1)
      kTty_Putc ('R');
    else
      kTty_Putc ('?');
  }  
  
  kTty_Putc ('\n');
}


// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context */
void kPg_Resolve (uint32_t address, uint32_t *table, int rights)
{
  int dir = (address >> 22) & 0x3ff;
  int tbl = (address >> 12) & 0x3ff;

  if (table[dir] == 0) {
    table[dir] = kPg_AllocPage() | rights;

    if (table != TABLE_DIR_THR)
      TABLE_DIR_THR[dir] = table[dir];

    memset (TABLE_PAGE_TH(dir), 0, PAGE_SIZE);
  }

  if (TABLE_PAGE_TH(dir)[tbl] == 0) {
    TABLE_PAGE_TH(dir)[tbl] = kPg_AllocPage() | rights;
    memset ((void*)(address & ~(PAGE_SIZE-1)), 0, PAGE_SIZE);
  }
}


// ---------------------------------------------------------------------------
int kPg_Fault (uint32_t address)
{
  // kprintf ("PAGE_FAULT\n");

  if (address < USR_SPACE_BASE)
    kpanic ("PG NOT ALLOWED <%x> \n", address);
  else if (address < USR_SPACE_LIMIT) {

  } else if (address < 0xffffc000) 
    kPg_Resolve (address, TABLE_DIR_KRN, PG_KERNEL_ONLY);
  else 
    kpanic ("PG NOT ALLOWED <%x> \n", address);

  return __noerror();
}




