/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to handle devices operations.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Attach an inode to its parent. */
static int attach_inode (kInode_t* ino, kInode_t* dir, const char* name)
{
  int k;
  kInode_t* cursor = dir->child_;
  assert (kislocked (&dir->lock_));
  assert (kislocked (&ino->lock_));
  ino->parent_ = dir;

  if (dir->child_ == NULL) {
    dir->child_ = ino;
    return __noerror ();
  }

  if (0 < strcmp(cursor->name_, name)) {
    ino->next_ = dir->child_;
    cursor->prev_ = ino;
    dir->child_ = ino;
    return __noerror ();
  }

  for (;;) {
    if (cursor->next_ == NULL) {
      ino->prev_ = cursor;
      cursor->next_ = ino;
      return __noerror ();
    }

    k = strcmp(cursor->next_->name_, name);

    if (k < 0 && cursor->next_ != NULL) {
      cursor = cursor->next_;
      continue;
    }

    if (k == 0) {
      return __seterrno(EEXIST);
    }

    ino->prev_ = cursor;
    ino->next_ = cursor->next_;
    cursor->next_->prev_ = ino;
    cursor->next_ = ino;
    return __noerror ();
  }
}


// ---------------------------------------------------------------------------
/** Detach an inode from its parent. */
static int detach_inode (kInode_t* ino)
{
  
}


// ---------------------------------------------------------------------------
/** Try to add a new inode on the VFS tree. */
kInode_t* register_inode (const char* name, kInode_t* dir, kStat_t* stat)
{
  static id_t auto_incr = 0;
  assert (PARAM_FILENAME(name));
  assert (dir != NULL && stat != NULL);
  assert (S_ISDIR (dir->stat_.mode_));
  assert (kislocked(&dir->lock_));

  kInode_t* ino = KALLOC(kInode_t);
  if (ino == NULL) {
    __seterrno (ENOMEM);
    return NULL;
  }

  ino->name_ = kcopystr(name);
  ino->dev_ = dir->dev_;
  ino->devinfo_ = dir->devinfo_; // @todo should be include on dev_ !
  memcpy (&ino->stat_, stat, sizeof(kStat_t));
  ino->stat_.dev_ = dir->stat_.dev_;
  ino->stat_.ino_ = ++auto_incr; // SYS_HANDLE();
  ino->stat_.mode_ &= (S_IALLUGO | S_IFMT);
  // klock(&dir->lock_);
  klock(&ino->lock_);
  if (attach_inode(ino, dir, name)) {
    kunlock (&dir->lock_);
    kunlock (&ino->lock_);
    kfree(ino);
    return NULL;
  }

  kunlock (&dir->lock_);
  return ino;
}


// ---------------------------------------------------------------------------
/** Release an inode form the inode cache. */
int unregister_inode (kInode_t* ino)
{
  return __seterrno (ENOSYS);
}


// ---------------------------------------------------------------------------
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes)
{
  return __seterrno (ENOSYS);
}


// ---------------------------------------------------------------------------
/** Call the bucket scavanger which will try to free cached page. */
int scavenge_bucket(int pages)
{
  return __seterrno (ENOSYS);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
