#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
#include <kernel/uparams.h>
#include <stdio.h>


// ===========================================================================
ssize_t kstm_read_block (kStream_t* stream, void* buf, size_t length, off_t off)
{
  uint32_t page;
  ssize_t count = 0;
  // size_t lg = 1;
  // kAddSpace_t* addSpace = kCPU.current_->process_->memSpace_;

  while (length > 0) {

    // kVma_t* vma = kvma_look_ino (addSpace, stream->ino_, off);
    // assert (vma != NULL);
    // TRY MANUAL MAPPING (may be not need to try that)
    // COMPUTE SIZE...
    ssize_t poff = ALIGN_DW(off, PAGE_SIZE);
    kfs_map (stream->ino_, poff, &page);

    void* address = kpg_temp_page (&page);
    ssize_t lg = PAGESIZE + poff - off;
    lg = MIN (lg, stream->ino_->stat_.length_ - off);
    if (lg == 0)
      break;

    if (KLOG_RW) kprintf ("read %s [%d+%d]\n", stream->ino_->name_, off, lg);

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
ssize_t kstm_write_block (kStream_t* stream, void* buf, size_t length, off_t off)
{

  ssize_t count = 0;
  kAddSpace_t* addSpace = kCPU.current_->process_->memSpace_;

  while (length > 0) {

    kVma_t* vma = kvma_look_ino (addSpace, stream->ino_, off);
    assert (vma != NULL);

    // TRY MANUAL MAPPING (may be not need to try that)

    // COMPUTE SIZE...
    void* address = (void*)((size_t)vma->base_ + off - vma->offset_);
    size_t lg = MIN (length, (vma->limit_ - vma->base_) + vma->offset_ - off);

    if (KLOG_RW) kprintf ("write %s [%d+%d]\n", stream->ino_->name_, off, lg);

    memcpy (address, buf, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;
    if (stream->ino_->stat_.length_ < (size_t)off)
      stream->ino_->stat_.length_ = (size_t)off;

    // kpg_sync_stream (vma, address);
  }

  stream->position_ = off;
  if (KLOG_RW) kprintf ("fs] write block [%d]\n", count);
  return count;
}
