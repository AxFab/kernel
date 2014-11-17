#include <kernel/streams.h>
#include <kernel/vfs.h>


#include <kernel/info.h>

int font64_paint(kTerm_t* term, kLine_t* style, int row);
void font64_clean(kTerm_t* term);


// ---------------------------------------------------------------------------
kTerm_t* term_open (kInode_t* ino);
void term_close (kTerm_t* term);
void term_scroll (kTerm_t* term, int count);
void term_redraw(kTerm_t* term);
void term_frame(kTerm_t* term, void* pixels, int width, int height, int line, 
                int (*paint)(kTerm_t*, kLine_t*, int), void (*clear)(kTerm_t*));

// ---------------------------------------------------------------------------
kInode_t* term_create (void* pixels, int width, int height, int line)
{
  char no[10];

  snprintf (no, 10, "tty%d", kSYS.autoPipe_++);
  kInode_t* ino = create_inode(no, kSYS.pipeNd_, S_IFTTY | 0600, width * height);
  assert (ino != NULL);

  term_open(ino);

  term_frame (ino->term_, pixels, width, height, line, font64_paint, font64_clean);
  return ino;
}




