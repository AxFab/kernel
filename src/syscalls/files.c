#include <kernel/inodes.h>
#include <kernel/scheduler.h>
#include <kernel/memory.h>
#include <stdio.h>
#include <kernel/info.h>
#include <kernel/uparams.h>

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

  if (fd < 0 || fd >= proc->streamCap_) {
    __seterrno (EBADF);
    return NULL;
  }

  klock (&proc->lock_, LOCK_SYSFILE_FD);
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

  kfs_grab (ino); // TODO if failed !? free readFD cause probably a lock stuff
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
static ssize_t read_block (kStream_t* stream, void* buf, size_t length, off_t off)
{
  ssize_t count = 0;
  kAddSpace_t* addSpace = kCPU.current_->process_->memSpace_;

  while (length > 0) {

    kVma_t* vma = kvma_look_ino (addSpace, stream->ino_, off);
    assert (vma != NULL);

    // TRY MANUAL MAPPING (may be not need to try that)

    // COMPUTE SIZE...
    void* address = (void*)((size_t)vma->base_ + off - vma->offset_);
    size_t lg = length;

    memcpy (buf, address, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;
  }

  return count;
}


// ---------------------------------------------------------------------------
static ssize_t write_block (kStream_t* stream, void* buf, size_t length, off_t off)
{

  ssize_t count = 0;
  kAddSpace_t* addSpace = kCPU.current_->process_->memSpace_;

  while (length > 0) {

    kVma_t* vma = kvma_look_ino (addSpace, stream->ino_, off);
    assert (vma != NULL);

    // TRY MANUAL MAPPING (may be not need to try that)

    // COMPUTE SIZE...
    void* address = (void*)((size_t)vma->base_ + off - vma->offset_);
    size_t lg = length;

    memcpy (address, buf, lg);
    buf = (char*)buf + lg;
    length -= lg;
    off += lg;
    count += lg;

    // kpg_sync_stream (vma, address);
  }

  kprintf ("fs] write block [%d]\n", count);
  return count;
}


// ---------------------------------------------------------------------------
static ssize_t read_stream (kStream_t* stream, void* buf, size_t length)
{
  __seterrno (ENOSYS);
  return -1;
}


// ---------------------------------------------------------------------------
static ssize_t write_stream (kStream_t* stream, void* buf, size_t length)
{
  __seterrno (ENOSYS);
  return -1;
}


// ===========================================================================
/**
 *  \note Read and write are copy-code, try to merge them.
 */
ssize_t kSys_Read (int fd, void* buf, size_t length, off_t off)
{
  if (KLOG_SYC) kprintf ("syscall %d] kSys_Read (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, fd, buf, length, off);

  kStream_t* stream = kSys_GetFd (fd, R_OK);
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
      return read_block (stream, buf, length, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return read_stream (stream, buf, length);

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
ssize_t kSys_Write (int fd, void* buf, size_t length, off_t off)
{
  if (KLOG_SYC) kprintf ("syscall %d] kSys_Write  (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, fd, buf, length, off);

  kStream_t* stream = kSys_GetFd (fd, W_OK);
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
      return write_block (stream, buf, length, off);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return write_stream (stream, buf, length);

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
int kSys_Mknod(int dirfd, const char *path, mode_t mode)
{
  // FIXME put in other file
  __seterrno (ENOSYS);
  return -1;
}
// ---------------------------------------------------------------------------
/** Open and possibly create a file
 *
 * \error EACCES The requested access to the file or an element of its path is not allowed.
 * \error EDQUOT Where O_CREAT is specified, the file does not exist, and the user's quota has been exhausted.
 * \error EEXIST path already exists and O_CREAT and O_EXCL were used.
 * \error EFAULT path points outside your accessible address space.
 * \error EINVAL Invalid value in flags.
 * \error EISDIR path refers to a directory and the access requested involved writing
 * \error ELOOP  Too many symbolic links were encountered in resolving path.
 * \error EMFILE The process already has the maximum number of files open.
 * \error ENAMETOOLONG path was too long.
 * \error ENFILE The system limit on the total number of open files has been reached.
 * \error ENOENT O_CREAT is not set and the named file does not exist.
 * \error ENOMEM Insufficient kernel memory was available.
 * \error ENOSPC path was to be created but the device containing path has no room for the new file.
 * \error ENOTDIR A component used as a directory in path is not.
 * \error EOPNOTSUPP The filesystem containing path does not support O_TMPFILE.
 * \error EOVERFLOW path refers to a regular file that is too large to be opened.
 * \error EPERM  The O_NOATIME flag was specified, but the effective user ID of the caller did not match the owner of the file and the caller was not privileged (CAP_FOWNER).
 * \error EROFS  path refers to a file on a read-only filesystem and write access was requested.
 * \error ETXTBSY path refers to an executable image which is currently being executed and write access was requested.
 * \error EWOULDBLOCK The O_NONBLOCK flag was specified, and an incompatible lease was held on the file
 * \error EBADF  dirfd is not a valid file descriptor (flags O_RELATIVE).
 * \error ENOTDIR dirfd is a file descriptor referring to a file other than a directory (flags O_RELATIVE).
 */
int kSys_Open(int dirfd, const char *path, int flags, mode_t mode)
{
  if (KLOG_SYC) kprintf ("syscall %d] kSys_Open  (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, dirfd, path, flags, mode);

  kInode_t* dir = NULL;
  kInode_t* ino = NULL;
  kStream_t* dirStm = NULL;

  if (!kUserParam_String (kCPU.current_->process_->memSpace_, path, PATH_MAX))
    return -1;

  // FIXME Check flags validity
  if (dirfd >= 0) {
    dirStm = kSys_GetFd (dirfd, R_OK);
    if (dirStm) {
      dir = dirStm->ino_;
      if (!S_ISDIR(dir->stat_.mode_)) {
          __seterrno (ENOTDIR);
          return -1;
      }
    }
    return -1;
  }

  ino = kfs_lookup (path, dir);
  if (ino == NULL) {

    if (__geterrno() == ELOOP || __geterrno() == ENOTDIR)
      return -1;

    if ((flags & O_CREAT) == 0) {
      __seterrno (ENOENT);
      return -1;
    }

    if (kSys_Mknod(dirfd, path, mode | (flags & O_DIRECTORY ? S_IFDIR : S_IFREG)))
      return -1;

  } else if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
    __seterrno (EEXIST);
    return -1;
  }

  if ((flags & O_DIRECTORY) && !S_ISDIR(ino->stat_.mode_)) {
    __seterrno (ENOTDIR);
    return -1;
  }

  // FIXME check rights
  return kSys_SetFd (ino, flags);
}


// ---------------------------------------------------------------------------
/**
 */
int kSys_Close (int fd)
{
  return kSys_ClearFd (fd);
}

