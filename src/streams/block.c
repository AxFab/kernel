#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/vfs.h>
#include <kernel/uparams.h>
#include <stdio.h>


// ===========================================================================
ssize_t kstm_read_block (kStream_t* stream, void* buf, size_t length, off_t off)
{
  uint32_t page;
  ssize_t count = 0;

  while (length > 0) {

    ssize_t poff = ALIGN_DW(off, PAGE_SIZE);
    inode_page (stream->ino_, poff, &page);

    void* address = kpg_temp_page (&page);
    address = ((char*)address) + (off - poff);
    ssize_t lg = PAGE_SIZE + poff - off;
    lg = MIN (lg, (ssize_t)length);
    lg = MIN (lg, (ssize_t)(stream->ino_->stat_.length_ - off));
    if (lg == 0)
      break;

    if (KLOG_RW) kprintf ("fs] read %s [%d+%d]\n", stream->ino_->name_, off, lg);

    memcpy (buf, address, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;
  }

  stream->position_ = off;
  return count;
}


// ---------------------------------------------------------------------------
ssize_t kstm_write_block (kStream_t* stream, const void* buf, size_t length, off_t off)
{
  uint32_t page;
  ssize_t count = 0;

  while (length > 0) {

    ssize_t poff = ALIGN_DW(off, PAGE_SIZE);
    inode_page (stream->ino_, poff, &page);

    void* address = kpg_temp_page (&page);
    address = ((char*)address) + (off - poff);
    ssize_t lg = PAGE_SIZE + poff - off;
    lg = MIN (lg, (ssize_t)length);
    if (lg == 0)
      break;

    if (KLOG_RW) kprintf ("fs] write %s [%d+%d]\n", stream->ino_->name_, off, lg);

    memcpy (address, buf, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;
    if (stream->ino_->stat_.length_ < (size_t)off)
      stream->ino_->stat_.length_ = (size_t)off;

    // FIXME push info that this inode page need sync
  }

  stream->position_ = off;
  if (KLOG_RW) kprintf ("fs] write block [%d]\n", count);
  return count;
}
