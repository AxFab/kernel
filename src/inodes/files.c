/*
 *      This file is part of the KERNEL project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used for common file operations.
 */
#include <inodes.h>
#include <kcpu.h>

// ---------------------------------------------------------------------------

int kFs_Open(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_inc_i32(&ino->readers_);
  return __noerror();
}

// ---------------------------------------------------------------------------

int kFs_Close(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_dec_i32(&ino->readers_);
  return __noerror();
}

// ===========================================================================

// ---------------------------------------------------------------------------
/** Create a new inode.
 *  The request is first send to file system.
 *  If accepted, the vfs register the new file.
 */
kInode_t* kFs_MkNode(const char* name, kInode_t* dir, kStat_t* stat)
{
  int err;
  kInode_t* ino;

  if (!dir->fs_->create) {
    __seterrno(EROFS);
    return NULL;
  }

  MOD_ENTER;
  err = dir->fs_->create(name, dir, stat);
  MOD_LEAVE;

  if (err) {
    __seterrno(err);
    return NULL;
  }

  ino = kFs_Register (name, dir, stat);
  if (ino)
    kunlock (&ino->lock_);
  return ino;
}

// ---------------------------------------------------------------------------

int kFs_Delete(kInode_t* ino)
{
  return __seterrno(ENOSYS);
}

// ---------------------------------------------------------------------------

ssize_t kFs_Read(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (ino != NULL && buffer != NULL);

  if (ino->fs_->read == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  if (KLOG_VFS) kprintf ("FS] Read '%s' on LBA %d using %x\n", ino->name_, offset, ino->fs_->read);
  int err = ino->fs_->read (ino, buffer, length, offset);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return length;
}




