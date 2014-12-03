/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to handle file system entries modification.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Request the file system for the creation of a new inode. */
kInode_t* create_inode(const char* name, kInode_t* dir, mode_t mode, size_t lg)
{
  assert (PARAM_FILENAME(name));
  assert (PARAM_KOBJECT(dir, kInode_t));

  if (!dir->dev_->create) {
    __seterrno(EROFS);
    return NULL;
  }
  
  kStat_t stat = { 0 };
  stat.atime_ = stat.ctime_ = stat.mtime_ = time (NULL);
  int rg = mode & 0777;
  int tp = mode & S_IFMT;
  if (!S_ISREG(tp) && !S_ISDIR(tp) && !S_ISFIFO(tp) && !S_ISTTY(tp)){
    __seterrno(EINVAL);
    return NULL;
  }

  stat.mode_ = rg | tp;
  stat.length_ = lg;

  klock(&dir->lock_);
  MODULE_ENTER(&dir->lock_, &dir->dev_->lock_);
  int err = dir->dev_->create(name, dir, &stat);
  MODULE_LEAVE(&dir->lock_, &dir->dev_->lock_);

  if (err) {
    __seterrno(err);
    return NULL;
  }

  kInode_t* ino = register_inode (name, dir, &stat);
  if (ino)
    kunlock (&ino->lock_);
  return ino;
}


// ---------------------------------------------------------------------------
/** Request the file system to remove an inode. */
int remove_inode(kInode_t* ino)
{
  return __seterrno(ENOSYS);
}


// ---------------------------------------------------------------------------
/** Request the file system to update the inode metadata. */
int chmeta_inode(kInode_t* ino, kStat_t* stat)
{
  return __seterrno(ENOSYS);
}


// ---------------------------------------------------------------------------
/** Request the file system to change the path of the inode. */
int rename_inode(kInode_t* ino, const char* name, kInode_t* dir)
{
  return __seterrno(ENOSYS);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
