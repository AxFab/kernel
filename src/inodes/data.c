/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used for common file operations.
 */
#include <kernel/inodes.h>


// ===========================================================================
/**
 */
int kfs_feed(kInode_t* ino, void* buffer, size_t length, off_t offset)
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


// ---------------------------------------------------------------------------
int kfs_sync(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (ino != NULL && buffer != NULL);

  if (ino->fs_->write == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  if (KLOG_VFS) kprintf ("FS] Write '%s' on LBA %d using %x\n", ino->name_, offset, ino->fs_->write);
  int err = ino->fs_->write (ino, buffer, length, offset);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return length;
}


// ---------------------------------------------------------------------------

int kfs_map (kInode_t*ino, off_t offset, uint32_t* page, int* mustRead)
{
  *page = kPg_AllocPage();
  *mustRead = 1;
  return __noerror();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
