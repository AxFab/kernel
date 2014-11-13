#include <kernel/streams.h>
#include <kernel/inodes.h>


#include <kernel/info.h>

int font64_paint(kTerm_t* term, kLine_t* style, int row);
void font64_clean(kTerm_t* term);

// ---------------------------------------------------------------------------
kInode_t* term_create (void* pixels, int width, int height)
{
  char no[10];

  kStat_t stat = { 0 };
  stat.mode_ = S_IFTTY | 0600;
  stat.atime_ = stat.ctime_ = stat.mtime_ = time (NULL);
  // stat.length_ = length;

  snprintf (no, 10, "tty%d", kSYS.autoPipe_++);
  kInode_t* ino = kfs_mknod(no, kSYS.pipeNd_, &stat);
  assert (ino != NULL);

  term_open(ino);

  term_frame (ino->term_, pixels, width, height, font64_paint, font64_clean);
  return ino;
}




