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
#include <kernel/streams.h>
#include <kernel/uparams.h>
#include <kernel/inodes.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>


// ===========================================================================
kStream_t* kstm_get_fd (int fd, int mode)
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
int kstm_set_fd (kInode_t* ino, int flags)
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
int kstm_clear_fd (int fd)
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
int kstm_open(int dirfd, const char *path, int flags, mode_t mode)
{
  if (KLOG_SYC) kprintf ("syscall %d] kstm_open  (%d, 0x%x, %d, %d);\n", kCPU.current_->process_->pid_, dirfd, path, flags, mode);

  kInode_t* dir = NULL;
  kInode_t* ino = NULL;
  kStream_t* dirStm = NULL;

  if (!kUserParam_String (kCPU.current_->process_->memSpace_, path, PATH_MAX))
    return -1;

  // FIXME Check flags validity
  if (dirfd >= 0) {
    dirStm = kstm_get_fd (dirfd, R_OK);
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

    if (kstm_mknod(dirfd, path, mode | (flags & O_DIRECTORY ? S_IFDIR : S_IFREG)))
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
  return kstm_set_fd (ino, flags);
}


// ---------------------------------------------------------------------------
/**
 */
int kstm_close (int fd)
{
  return kstm_clear_fd (fd);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
