#include <kernel/stream.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <kernel/params.h>

// ===========================================================================
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t stream_read (int fd, void* buf, size_t length)
{
  kStream_t* stream = stream_get (fd, O_RDONLY);
  if (stream == NULL) 
    return -1;

  if (!PARAM_USER_BUFFER(kCPU.current_->process_->memSpace_, buf, length))
    return -1;

  assert (stream->read != NULL);
  return stream->read (stream, buf, length);
}


// ---------------------------------------------------------------------------
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t stream_write (int fd, const void* buf, size_t length)
{
  kStream_t* stream = stream_get (fd, O_WRONLY);
  if (stream == NULL) 
    return -1;

  if (!PARAM_USER_BUFFER(kCPU.current_->process_->memSpace_, buf, length))
    return -1;

  assert (stream->write != NULL);
  return stream->write (stream, buf, length);
}


// ---------------------------------------------------------------------------
/**
 */
off_t stream_seek(int fd, off_t offset, int whence)
{
  kStream_t* stream = stream_get (fd, R_OK | W_OK);
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


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
