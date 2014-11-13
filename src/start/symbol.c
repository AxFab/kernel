#include <kernel/core.h>
#include <kernel/inodes.h>

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
  char *tmp = kalloc (ALIGN_UP (lg, PAGE_SIZE));
  char *str = kalloc (512);
  char *sym = kalloc (512);
  char *add = kalloc (20);

  kfs_grab (ino);
  klock(&ino->lock_);
  feed_inode (ino, tmp, ALIGN_UP (lg, ino->stat_.block_) / ino->stat_.block_, 0);
  kunlock(&ino->lock_);

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

  kfree (tmp);
  kfree (str);
  kfree (sym);
  kfree (add);
  kfs_release (ino);
}

