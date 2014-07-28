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
#include <kernel/cpu.h>


// ===========================================================================
/** Create a new inode.
 *  The request is first send to file system.
 *  If accepted, the vfs register the new file.
 */
kInode_t* kfs_mknod(const char* name, kInode_t* dir, kStat_t* stat)
{
  int err;
  kInode_t* ino;

  assert (kcpu_state() == CPU_STATE_SYSCALL);

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

  ino = kfs_register (name, dir, stat);
  if (ino)
    kunlock (&ino->lock_);
  return ino;
}


// ---------------------------------------------------------------------------
/** Remove an inode from the file system */
int kfs_remove(kInode_t* ino)
{
  assert (kcpu_state() == CPU_STATE_SYSCALL);

  return __seterrno(ENOSYS);
}


// ---------------------------------------------------------------------------
/** */
int kfs_follow_link (kInode_t** ino, int* loopCount)
{
  assert (kcpu_state() == CPU_STATE_SYSCALL);

  (*loopCount)++;
  return __seterrno (ENOSYS);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
