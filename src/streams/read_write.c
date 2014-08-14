#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
#include <kernel/uparams.h>
#include <stdio.h>



// ===========================================================================
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t kstm_read (int fd, void* buf, size_t length, off_t off)
{
  if (KLOG_RW) kprintf ("syscall %d] kstm_read (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, fd, buf, length, off);

  kStream_t* stream = kstm_get_fd (fd, R_OK);
  if (stream == NULL) return -1;

  if (!kUserParam_Buffer(kCPU.current_->process_->memSpace_, buf, length))
    return -1;

  int type = stream->ino_->stat_.mode_ & S_IFMT;
  assert (type != S_IFLNK);
  switch (stream->ino_->stat_.mode_ & S_IFMT) {
    case S_IFREG:
    case S_IFBLK:
      if (off < 0)
        off = stream->position_;
      return kstm_read_block (stream, buf, length, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return kstm_read_pipe (stream, buf, length);

    case S_IFSOCK:
      __seterrno(ENOSYS);
      return -1;

    default:
      assert (!0);
      return -1;
  }
}


// ---------------------------------------------------------------------------
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t kstm_write (int fd, void* buf, size_t length, off_t off)
{
  if (KLOG_RW) kprintf ("syscall %d] kstm_write  (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, fd, buf, length, off);

  kStream_t* stream = kstm_get_fd (fd, W_OK);
  if (stream == NULL) return -1;

  if (!kUserParam_Buffer(kCPU.current_->process_->memSpace_, buf, length))
    return -1;

  int type = stream->ino_->stat_.mode_ & S_IFMT;
  assert (type != S_IFLNK);
  switch (stream->ino_->stat_.mode_ & S_IFMT) {
    case S_IFREG:
    case S_IFBLK:
      if (off < 0)
        // TODO think to O_APPEND
        off = stream->position_;
      return kstm_write_block (stream, buf, length, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return kstm_write_pipe (stream, buf, length);

    case S_IFSOCK:
      __seterrno(ENOSYS);
      return -1;

    default:
      assert (!0);
      return -1;
  }
}


// ---------------------------------------------------------------------------
/**
 */
off_t kstm_seek(int fd, off_t offset, int whence)
{
  kStream_t* stream = kstm_get_fd (fd, R_OK | W_OK);
  if (stream == NULL) return -1;

  int type = stream->ino_->stat_.mode_ & S_IFMT;
  assert (type != S_IFLNK);
  if (type == S_IFCHR || type == S_IFIFO || type == S_IFSOCK) {
    __seterrno(ESPIPE);
    return -1;
  } else if (type == S_IFDIR) {
      __seterrno(ENOSYS);
      return -1;
  }

  assert (type == S_IFREG || type == S_IFBLK);
  if (whence == SEEK_SET)
    stream->position_ = offset;
  else if (whence == SEEK_CUR)
    stream->position_ += offset;
  else if (whence == SEEK_END)
    stream->position_ = stream->ino_->stat_.length_ - offset;
  else {
    // if (whence == SEEK_DATA || whence == SEEK_HOLE)
    //   __seterrno (ENOSYS);
    // else
      __seterrno (EINVAL);
    return -1;
  }

  stream->position_ = MIN (stream->ino_->stat_.length_,
                        MAX(0, stream->position_));

  if (stream->position_ != (size_t)((off_t)stream->position_)) {
    __seterrno (EOVERFLOW);
    return -1;
  }

  return stream->position_;
}


