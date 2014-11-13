#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
#include <kernel/uparams.h>
#include <stdio.h>


// ===========================================================================
// ---------------------------------------------------------------------------
static inline kPipe_t* kstm_get_pipe (kStream_t* stream)
{
  if (stream->ino_->pipe_ == 0) {
    stream->ino_->pipe_ = KALLOC (kPipe_t);
    stream->ino_->pipe_->length_ = stream->ino_->stat_.length_;
  }

  return stream->ino_->pipe_;
}

// ===========================================================================

// ---------------------------------------------------------------------------
ssize_t kstm_read_pipe (kStream_t* stream, void* buf, size_t length)
{
  uint32_t page;
  kPipe_t* pipe = kstm_get_pipe(stream);
  ssize_t count = 0;

  while (length > 0) {

    if (pipe->outPen_ >= pipe->length_)
      pipe->outPen_ = 0;

    ssize_t poff = ALIGN_DW(pipe->outPen_, PAGE_SIZE);
    kfs_map (stream->ino_, poff, &page);
    if (KLOG_RW) kprintf ("PIPE R %s, at %d  [%x]\n", stream->ino_->name_, poff, page);
    void* address = kpg_temp_page (&page);
    address = ((char*)address) + (pipe->outPen_ - poff);

    ssize_t outCap = pipe->length_ - pipe->outPen_;
    outCap = MIN (outCap, (ssize_t)length);
    if (pipe->outPen_ > pipe->inPen_) {
      outCap = MIN (outCap, pipe->length_ - pipe->outPen_);
    } else {
      outCap = MIN (outCap, pipe->inPen_ - pipe->outPen_);
    }

    if (outCap == 0) {
      if (KLOG_RW) kprintf ("fs] read no available %s\n", stream->ino_->name_);
      // kevt_wait_available_data (pipe, W_OK);
      return count;
    }

    if (KLOG_RW) kprintf ("fs] read %s [%d+%d]\n", stream->ino_->name_, pipe->outPen_, outCap);

    memcpy (buf, address, outCap);
    length -= outCap;
    pipe->outPen_ += outCap;
    count += outCap;
    buf = ((char*)buf) + outCap;

    *((char*)buf) = '\0'; // FIXME segfault here
    // if (KLOG_RW) kprintf ("write #[%d;%d] {%s}\n", outCap, pipe->outPen_, "-");
  }

  if (length > 0)
    *((char*)buf) = '\0';

  return count;
}


// ---------------------------------------------------------------------------
ssize_t kstm_write_pipe (kStream_t* stream, void* buf, size_t length)
{
  uint32_t page;
  kPipe_t* pipe = kstm_get_pipe(stream);
  ssize_t count = 0;

  while (length > 0) {

    if (pipe->inPen_ >= pipe->length_)
      pipe->inPen_ = 0;

    ssize_t poff = ALIGN_DW(pipe->inPen_, PAGE_SIZE);
    kfs_map (stream->ino_, poff, &page);
    if (KLOG_RW) kprintf ("PIPE W %s, at %d  [%x]\n", stream->ino_->name_, poff, page);
    void* address = kpg_temp_page (&page);
    address = ((char*)address) + (pipe->inPen_ - poff);

    ssize_t outCap = pipe->length_ - pipe->inPen_;
    outCap = MIN (outCap, (ssize_t)length);
    if (pipe->outPen_ > pipe->inPen_) {
      outCap = MIN (outCap, pipe->outPen_ - pipe->inPen_ - 1);
    } else {
      outCap = MIN (outCap, pipe->length_ - pipe->outPen_);
    }

    if (outCap == 0) {
      if (KLOG_RW) kprintf ("fs] write no available %s\n", stream->ino_->name_);
      // kevt_wait_available_data (pipe, W_OK);
      return count;
    }

    if (KLOG_RW) kprintf ("fs] write %s [%d+%d]\n", stream->ino_->name_, pipe->inPen_, outCap);

    memcpy (address, buf, outCap);
    length -= outCap;
    pipe->inPen_ += outCap;
    count += outCap;
    buf = ((char*)buf) + outCap;

    // if (KLOG_RW) kprintf ("write #[%d;%d] {%s}\n", outCap, pipe->inPen_, "-");
  }

  return count;
}

// ---------------------------------------------------------------------------
ssize_t kstm_read_pipe_line  (kStream_t* stream, void* buf, size_t length)
{
  uint32_t page;
  kPipe_t* pipe = kstm_get_pipe(stream);
  ssize_t count = 0;

  while (length > 0) {

    if (pipe->outPen_ >= pipe->length_)
      pipe->outPen_ = 0;

    ssize_t poff = ALIGN_DW(pipe->outPen_, PAGE_SIZE);
    kfs_map (stream->ino_, poff, &page);
    if (KLOG_RW) kprintf ("PIPE R %s, at %d  [%x]\n", stream->ino_->name_, poff, page);
    void* address = kpg_temp_page (&page);
    address = ((char*)address) + (pipe->outPen_ - poff);

    ssize_t outCap = pipe->length_ - pipe->outPen_;
    ssize_t nextEndl = 0;

    while (*(((char*)address) + nextEndl) != '\n' &&
        *(((char*)address) + nextEndl) != '\0' &&
        pipe->inPen_ + nextEndl < pipe->length_) {
      nextEndl++;
    }
    if (*(((char*)address) + nextEndl) != '\n') {
      // FIXME need to look where is the boundary, evantually read buffer start..
      // kevt_wait_available_data (tty, R_OK);
      return count;
    }

    ++nextEndl;
    outCap = MIN (outCap, nextEndl);
    outCap = MIN (outCap, (ssize_t)length);
    if (pipe->outPen_ > pipe->inPen_) {
      outCap = MIN (outCap, pipe->length_ - pipe->outPen_);
    } else {
      outCap = MIN (outCap, pipe->inPen_ - pipe->outPen_);
    }

    if (outCap == 0) {
      if (KLOG_RW) kprintf ("fs] read no available %s\n", stream->ino_->name_);
      // kevt_wait_available_data (pipe, R_OK);
      return count;
    }

    if (KLOG_RW) kprintf ("fs] read %s [%d+%d]\n", stream->ino_->name_, pipe->outPen_, outCap);

    memcpy (buf, address, outCap);
    length -= outCap;
    pipe->outPen_ += outCap;
    count += outCap;
    buf = ((char*)buf) + outCap;
    if (((char*)buf)[-1] == '\n')
      break;
    // if (KLOG_RW) kprintf ("write #[%d;%d] {%s}\n", outCap, pipe->outPen_, "-");
  }

  if (length > 0)
    *((char*)buf) = '\0';

  return count;
}


// ---------------------------------------------------------------------------
ssize_t kstm_available_data_pipe (kStream_t* stream)
{
  uint32_t page;
  kPipe_t* pipe = kstm_get_pipe(stream);
  if (pipe->outPen_ >= pipe->length_)
    pipe->outPen_ = 0;

  ssize_t poff = ALIGN_DW(pipe->outPen_, PAGE_SIZE);
  kfs_map (stream->ino_, poff, &page);
  if (KLOG_RW) kprintf ("PIPE R %s, at %d  [%x]\n", stream->ino_->name_, poff, page);
  void* address = kpg_temp_page (&page);
  address = ((char*)address) + (pipe->outPen_ - poff);

  ssize_t intCap = pipe->length_ - pipe->outPen_;
  ssize_t nextEndl = 0;
  while (*(((char*)address) + nextEndl) != '\n' &&
      *(((char*)address) + nextEndl) != '\0' &&
      pipe->inPen_ + nextEndl < pipe->length_) {
    nextEndl++;
  }

  if (*(((char*)address) + nextEndl) != '\n') {
    // FIXME need to look where is the boundary, evantually read buffer start..
    return 0;
  }

  ++nextEndl;
  intCap = MIN (intCap, nextEndl);
  return intCap;
}

// ---------------------------------------------------------------------------
kStream_t* kstm_create_pipe (int flags, size_t length)
{
  char no[10];
  time_t now = time (NULL);
  length = ALIGN_UP (length, PAGE_SIZE);
  kStat_t stat = { 0, S_IFIFO | 0600, 0, 0, length, 0, now, now, now, 0, 0, 0 };
  snprintf (no, 10, "p%d", kSYS.autoPipe_++);
  kInode_t* ino = kfs_mknod(no, kSYS.pipeNd_, &stat);
  assert (ino != NULL);

  ino->fifo_ = KALLOC(kFifo_t);
  ino->fifo_->size_ = length;
  kStream_t* stream = KALLOC(kStream_t);
  stream->ino_ = ino;
  stream->flags_ = 0;

  if ((flags & O_ACCMODE) == O_RDONLY)        stream->flags_ = R_OK;
  else if ((flags & O_ACCMODE) == O_WRONLY)   stream->flags_ = W_OK;
  else                                        stream->flags_ = R_OK | W_OK;
  stream->flags_ |= (flags & O_STATMSK);

  return stream;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
