/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Implements system calls: File Management.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/sysapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>
#include <smkos/file.h>
#include <smkos/fcntl.h>

int sys_check_pathname(const char *path)
{
#if !defined(_FS_UM)
  int lg;
  int max;
  kMemArea_t *area = area_find(&kCPU.current_->process_->mspace_, (size_t)path);
  if (area == NULL)
    return __seterrno(EFAULT);
  max = MIN(area->limit_ - (size_t)path, PATH_MAX);
  lg = strnlen(path, max);
  if (lg >= max)
    return __seterrno(EINVAL);
#endif
  return __seterrno(0);
}

int sys_open(const char *path, int dirFd, int flags, int mode)
{
  kInode_t *dir = NULL;
  kInode_t *ino = NULL;
  char* name = NULL;
  kResx_t *resx;
  int lg;

  /* Check arguments */
  if (path == NULL || sys_check_pathname(path))
    return -1;


  /* Look for dir file descritpor */
  if (dirFd > 0) {
    resx = process_get_resx (kCPU.current_->process_, dirFd, 0);
    if (resx == NULL)
      return EBADF;
    dir = resx->ino_;
  }

  /* Look for path */
  if (path != NULL)
    ino = search_inode (path, dir, flags);

  if (ino == NULL) {
    if (flags & O_CREAT) {
      if (dir == NULL)
        dir = kCPU.current_->process_->session_->workingDir_;

      lg = strlen(path)+1;
      name = (char*)kalloc(lg);
      dir = search_last_parent (path, dir, flags, name, lg);
      ino = create_inode(name, dir, mode, 0);
      kfree(name);
      if (ino == NULL)
        return -1;
    } else {
      __seterrno(ENOENT);
      return -1;
    }
  } else if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
    __seterrno(EEXIST);
    return -1;
  }

  klock (&kCPU.current_->process_->lock_);
  resx = process_set_resx(kCPU.current_->process_, ino, 0); // O_RDLY or O_WRLY
  kunlock (&kCPU.current_->process_->lock_);
  return resx->fdNd_.value_;
}

int sys_close(int fd)
{
  return process_close_resx(kCPU.current_->process_, fd);
}

int sys_write(int fd, const void *data, size_t lg, off_t off)
{
  kResx_t *resx;

  if (lg > FBUFFER_MAX) {
    __seterrno(ENOSYS);
    return -1;
  }

  resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);

  if (resx == NULL)
    return -1;

  if (resx->ino_->subsys_) {
    mtx_lock(&resx->ino_->subsys_->mutex_);
    lg = resx->ino_->subsys_->write(data, lg);
    mtx_unlock(&resx->ino_->subsys_->mutex_);
    return lg;
  }

  switch (resx->type_) {
  case S_IFBLK:
  case S_IFREG:
    // return fs_block_write();
    return -1;

  case S_IFCHR:
  case S_IFIFO:
    return fs_pipe_write (resx->ino_, data, lg);

  case S_IFDIR:
    __seterrno(EISDIR);
    return -1;

  default:
    assert(resx->type_ == 0);
    __seterrno(EBADF);
    return -1;
  }
}

ssize_t sys_read(int fd, void *data, size_t lg, off_t off)
{
  ssize_t ret = 0;
  kResx_t *resx;

  // kprintf ("[Sr]");
  if (lg > FBUFFER_MAX) {
    __seterrno(ENOSYS);
    return -1;
  }

  resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);

  if (resx == NULL)
    return -1;

  switch (resx->type_) {
  case S_IFBLK:
  case S_IFREG:
    if (off >= 0)
      resx->seek_ = off;

    ret = fs_reg_read(resx->ino_, data, lg, resx->seek_);

    if (ret > 0)
      resx->seek_ += ret;

    return ret;

  case S_IFCHR:
  case S_IFIFO:
    ret = fs_pipe_read (resx->ino_, data, lg);

    if (ret == 0) {
      return -1;
    }

    return ret;

  case S_IFDIR:
    __seterrno(EISDIR);
    return -1;

  default:
    assert(resx->type_ == 0);
    __seterrno(EBADF);
    return -1;
  }
}

// int access(const char *pathname, int mode);
int sys_access (const char *path, int dirFd, int mode, int flags)
{
  // int err;
  kInode_t *ino = NULL;
  kResx_t *resx;

  // if (err = sys_check_pathname(path))
  //   return err;

  if (dirFd > 0) {
    resx = process_get_resx (kCPU.current_->process_, dirFd, 0);

    if (resx == NULL)
      return EBADF;

    ino = resx->ino_;
  }

  ino = search_inode (path, ino, flags);

  if (ino == NULL)
    return __geterrno();

  // R_OK, W_OK, X_OK (F_OK is explicit)
  // @todo test_capacity (mode);
  return 0;
}

