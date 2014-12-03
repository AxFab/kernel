/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Creation of file descriptors.
 */
#include <kernel/stream.h>
#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <kernel/params.h>
#include <kernel/info.h>


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
 *
 * FIXME ensure that we only wr-open file that are on a writable file system.
 */
kStream_t* stream_open (int dirfd, const char* path, int flags, int mode)
{
  kInode_t* dir = NULL;
  kInode_t* ino = NULL;
  kStream_t* dirStm = NULL;

  if (!PARAM_USER_STRING(kCPU.current_->process_->memSpace_, path, PATH_MAX))
    return NULL;

  // FIXME Check flags validity
  if (dirfd >= 0) {
    dirStm = stream_get (dirfd, R_OK);
    if (dirStm) {
      dir = dirStm->ino_;
      if (!S_ISDIR(dir->stat_.mode_)) {
          __seterrno (ENOTDIR);
          return NULL;
      }
    }
    return NULL;
  }

  ino = search_inode (path, dir);
  if (ino == NULL) {

    if (__geterrno() != ENOENT)
      return NULL;

    if ((flags & O_CREAT) == 0)
      return NULL;

    return stream_create(dirfd, path, flags, (mode & S_IALLUGO) | (flags & O_DIRECTORY ? S_IFDIR : S_IFREG));

  } else if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
    __seterrno (EEXIST);
    return NULL;
  }

  if ((flags & O_DIRECTORY) && !S_ISDIR(ino->stat_.mode_)) {
    __seterrno (ENOTDIR);
    return NULL;
  }

  // FIXME check rights
  return stream_set (ino, flags);
}


// ---------------------------------------------------------------------------
/**   */
kStream_t* stream_create (int dirfd, const char* path, int flags, int mode)
{
  kInode_t* dir = NULL;
  kInode_t* ino = NULL;
  kStream_t* dirStm = NULL;

  // FIXME Check flags validity
  if (dirfd >= 0) {
    dirStm = stream_get (dirfd, R_OK);
    if (dirStm) {
      dir = dirStm->ino_;
      if (!S_ISDIR(dir->stat_.mode_)) {
          __seterrno (ENOTDIR);
          return NULL;
      }
    }
    return NULL;
  }

  if (dir == NULL)
    dir = kSYS.rootNd_; // FIXME Hey, No!

  // kprintf ("create stream %x, %s, %d, %x\n", dir, path, flags, mode);
  // FIXME slit path for no directory
  // FIXME check rights
  // FIXME check the mode

  ino = create_inode(path, dir, mode, 0);
  if (ino == NULL) {
    return NULL;
  }

  return stream_set (ino, flags);
}


// ---------------------------------------------------------------------------
/**   */
int stream_close (int fd)
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


// ---------------------------------------------------------------------------
static kStream_t* stream_new (kProcess_t* proc)
{
  int i;
  kStream_t** openFd = proc->openStreams_;

  for (i = 0; i < proc->streamCap_; i++) {
    if (openFd[i] == NULL) {
      openFd[i] = KALLOC (kStream_t);
      openFd[i]->fd_ = i;
      return openFd[i];
    }
  }

  if (proc->streamCap_ >= OPEN_MAX) {
    kunlock (&proc->lock_);
    __seterrno (ENFILE);
    return NULL;
  }

  proc->streamCap_ += 8;
  openFd = kalloc (sizeof(kStream_t*) * proc->streamCap_, 0);
  memcpy (openFd, proc->openStreams_,
    sizeof(kStream_t*) * (proc->streamCap_));
  openFd[i] = KALLOC (kStream_t);
  openFd[i]->fd_ = i;
  if (proc->openStreams_ != NULL)
    kfree(proc->openStreams_);
  proc->openStreams_ = openFd;
  return openFd[i];
}

static int stream_build (kStream_t* stm, kInode_t* ino, int flags)
{
  // Assert ino, flags
  assert (ino != NULL);
  
  inode_open (ino); // TODO if failed !? free readFD cause probably a lock stuff
  stm->ino_ = ino;
  int type = ino->stat_.mode_ & S_IFMT;
  switch (type) {
    case S_IFREG:
    case S_IFBLK:
      stm->read = block_read;
      stm->write = block_write;
      break;
    case S_IFDIR:
      stm->read = dir_data;
      stm->write = (void*)dir_data;
      break;
    case S_IFIFO:
      stm->read = fifo_read;
      stm->write = fifo_write;
      break;
    case S_IFTTY: 
      stm->read = term_read;
      stm->write = term_write;
      break;
    default:
      assert(!0);
  }

  // if ((flags & O_ACCMODE) == O_RDONLY)        stm->flags_ = R_OK;
  // else if ((flags & O_ACCMODE) == O_WRONLY)   stm->flags_ = W_OK;
  // else                                        stm->flags_ = R_OK | W_OK;
  stm->flags_ = flags; // & O_STATMSK;

  return 0;
}

int stream_tty (kProcess_t* proc, kInode_t* tty)
{
  assert (proc->streamCap_ == 0);
  assert (proc->openStreams_ == NULL);

  kStream_t** openFd = kalloc (sizeof(kStream_t*) * 8, 0);
  proc->openStreams_ = openFd;
  proc->streamCap_ = 8;
  openFd[0] = KALLOC(kStream_t);
  openFd[0]->fd_ = 0;
  openFd[1] = KALLOC(kStream_t);
  openFd[0]->fd_ = 1;
  openFd[2] = KALLOC(kStream_t);
  openFd[0]->fd_ = 2;

  stream_build (openFd[0], tty, O_RDONLY);
  stream_build (openFd[1], tty, O_WRONLY);
  stream_build (openFd[2], tty, O_WRONLY);
  return 0;
}

// ---------------------------------------------------------------------------
/**   */
kStream_t* stream_set (kInode_t* ino, int flags)
{
  kProcess_t* proc = kCPU.current_->process_;
  klock (&proc->lock_);

  kStream_t* stm = stream_new(proc);
  if (stm == NULL)
    return NULL;

  stream_build(stm, ino, flags);
  kunlock (&proc->lock_);
  return stm;
}


// ---------------------------------------------------------------------------
/**   */
kStream_t* stream_get (int fd, int mode)
{
  kStream_t* stream;
  kProcess_t* proc = kCPU.current_->process_;
  if (fd < 0 || fd >= proc->streamCap_) {
    __seterrno (EBADF);
    return NULL;
  }

  klock (&proc->lock_, LOCK_SYSFILE_FD);
  stream = proc->openStreams_[fd];
  if (stream == NULL) {
    __seterrno (EBADF);
  } else if (!(stream->flags_ & mode)) {
    kprintf (LOG, "Get mode %x, look for %x for [%d]\n", stream->flags_, mode, fd);
    __seterrno (EACCES);
    stream = NULL;
  }

  kunlock (&proc->lock_);
  return stream;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
