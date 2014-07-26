#include <inodes.h>
#include <scheduler.h>
#include <memory.h>
#include <stdio.h>
#include <kinfo.h>
#include <uparams.h>

struct kStream
{
  kInode_t*     ino_;
  size_t        position_;
  int           flags_;
};


// ===========================================================================
static kStream_t* kSys_GetFd (int fd, int mode)
{
  kStream_t* stream;
  kProcess_t* proc = kCPU.current_->process_;

  klock (&proc->lock_, LOCK_SYSFILE_FD);
  if (fd >= proc->streamCap_) {
    kunlock (&proc->lock_);
    __seterrno (EBADF);
    return NULL;
  }

  stream = proc->openStreams_[fd];
  if (stream == NULL || !(stream->flags_ & mode)) {
    kunlock (&proc->lock_);
    __seterrno (EBADF);
    return NULL;
  }

  kunlock (&proc->lock_);
  return stream;
}


// ---------------------------------------------------------------------------
static int kSys_SetFd (kInode_t* ino, int flags)
{
  int i;
  kProcess_t* proc = kCPU.current_->process_;
  kStream_t** openFd = proc->openStreams_;

  klock (&proc->lock_, LOCK_SYSFILE_FD);
  for (i = 0; i < proc->streamCap_; i++) {
    if (openFd[i] == NULL) {
      openFd[i] = KALLOC (kStream_t);
      break;
    }
  }

  if (i >= proc->streamCap_) {
    if (proc->streamCap_ >= OPEN_MAX) {
      kunlock (&proc->lock_);
      __seterrno (ENFILE);
      return -1;
    }

    proc->streamCap_ += 8;
    openFd = kalloc (sizeof(kStream_t*) * proc->streamCap_);
    memcpy (openFd, proc->openStreams_,
      sizeof(kStream_t*) * (proc->streamCap_));
    openFd[i] = KALLOC (kStream_t);
    proc->openStreams_ = openFd;
  }

  kFs_Open (ino); // TODO if failed !? free readFD cause probably a lock stuff
  openFd[i]->ino_ = ino;
  openFd[i]->flags_ = flags;

  kunlock (&proc->lock_);
  return i;
}


// ---------------------------------------------------------------------------
static int kSys_ClearFd (int fd)
{
  kProcess_t* proc = kCPU.current_->process_;

  klock (&proc->lock_, LOCK_SYSFILE_FD);
  if (fd >= proc->streamCap_) {
    kunlock (&proc->lock_);
    __seterrno (EBADF);
    return -1;
  }

  if (proc->openStreams_[fd] == NULL) {
    kunlock (&proc->lock_);
    __seterrno (EBADF);
    return -1;
  }

  // sync (ino); // Unless flags say so, we don't need to wait, it's already on cache
  kfree (proc->openStreams_[fd]);
  proc->openStreams_[fd] = NULL;
  kunlock (&proc->lock_);
  return 0;
}


// ===========================================================================
static ssize_t read_block (kStream_t* stream, size_t length, void* buf, off_t off)
{
  ssize_t count = 0;
  kAddSpace_t* addSpace = kCPU.current_->process_->memSpace_;

  while (length > 0) {

    kVma_t* vma = kVma_FindFile (addSpace, stream->ino_, off);
    assert (vma != NULL);

    // TRY MANUAL MAPPING (may be not need to try that)

    // COMPUTE SIZE...
    void* address = vma->base_;
    size_t lg = 1;

    memcpy (buf, address, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;
  }

  return count;
}


// ---------------------------------------------------------------------------
static ssize_t write_block (kStream_t* stream, size_t length, void* buf, off_t off)
{
  // TODO think to O_APPEND
}


// ---------------------------------------------------------------------------
static ssize_t read_stream (kStream_t* stream, size_t length, void* buf)
{
}


// ---------------------------------------------------------------------------
static ssize_t write_stream (kStream_t* stream, size_t length, void* buf)
{
}


// ===========================================================================
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t kSys_Read (int fd, size_t length, void* buf, off_t off)
{
  if (KLOG_SYC) kprintf ("syscall] kSys_Read (%d, %d, 0x%x, %d);", fd, length, buf, off);

  kStream_t* stream = kSys_GetFd (fd, R_OK);
  if (stream == NULL) return -1;

  if (!kUserParam_Buffer(kCPU.current_->process_->memSpace_, buf, length)) {
    __seterrno (EFAULT);
    return -1;
  }

  int type = stream->ino_->stat_.mode_ & S_IFMT;
  assert (type != S_IFLNK);
  switch (stream->ino_->stat_.mode_ & S_IFMT) {
    case S_IFREG:
    case S_IFBLK:
      if (off < 0)
        off = stream->position_;
      return read_block (stream, length, buf, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return read_stream (stream, length, buf);

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
ssize_t kSys_Write (int fd, size_t length, void* buf, off_t off)
{
  if (KLOG_SYC) kprintf ("syscall] kSys_Write  (%d, %d, 0x%x, %d);", fd, length, buf, off);

  kStream_t* stream = kSys_GetFd (fd, W_OK);
  if (stream == NULL) return -1;

  if (!kUserParam_Buffer(kCPU.current_->process_->memSpace_, buf, length)) {
    __seterrno (EFAULT);
    return -1;
  }

  int type = stream->ino_->stat_.mode_ & S_IFMT;
  assert (type != S_IFLNK);
  switch (stream->ino_->stat_.mode_ & S_IFMT) {
    case S_IFREG:
    case S_IFBLK:
      if (off < 0)
        off = stream->position_;
      return write_block (stream, length, buf, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return write_stream (stream, length, buf);

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
off_t kSys_Seek(int fd, off_t offset, int whence)
{
  kStream_t* stream = kSys_GetFd (fd, R_OK | W_OK);
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
    if (whence == SEEK_DATA || whence == SEEK_HOLE)
      __seterrno (ENOSYS);
    else
      __seterrno (EINVAL);
    return -1;
  }

  stream->position_ = MIN (stream->ino_->stat_.length_,
                        MAX(0, stream->position_));

  if (stream->position_ != (size_t)((off_t)stream->position_))
    return __seterrno (EOVERFLOW);

  return stream->position_;
}


// ---------------------------------------------------------------------------
/**
 */
int kSys_Open(int dirfd, const char *pathname, int flags, mode_t mode)
{
  kInode_t* dir = NULL;
  kInode_t* ino = NULL;
  kStream_t* dirStm = kSys_GetFd (dirfd, R_OK);
  if (dirStm) {
    dir = dirStm->ino_;
  }

  ino = kFs_LookFor (pathname, dir);
  if ((flags & O_DIRECTORY) && !(ino->stat_.mode_ & S_IFDIR)) {
    __seterrno (ENOTDIR);
    return -1;
  }

  // TODO check rights
  // TODO SET FLAGS
  return kSys_SetFd (ino, flags);
}


// ---------------------------------------------------------------------------
/**
 */
int kSys_Close (int fd)
{
  return kSys_ClearFd (fd);
}

