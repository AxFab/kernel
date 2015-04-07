/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to handle inode registeration, creation and deletion.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Attach an inode to its parent. */
static int attach_inode (kInode_t *ino, kInode_t *dir, const char *name)
{
  int k;
  kInode_t *cursor = dir->child_;
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
/** Try to add a new inode on the VFS tree. */
kInode_t *register_inode (const char *name, kInode_t *dir, kStat_t *stat)
{
  static id_t auto_incr = 1;
  assert (PARAM_FILENAME(name));
  assert (dir != NULL && stat != NULL);
  assert (S_ISDIR (dir->stat_.mode_));
  assert (kislocked(&dir->lock_));

  kInode_t *ino = KALLOC(kInode_t);

  if (ino == NULL) {
    __seterrno (ENOMEM);
    return NULL;
  }

  ino->name_ = kstrdup(name);
  ino->dev_ = dir->dev_;
  memcpy (&ino->stat_, stat, sizeof(kStat_t));
  ino->stat_.dev_ = dir->stat_.dev_;
  ino->stat_.ino_ = ++auto_incr; // SYS_HANDLE();
  ino->stat_.mode_ &= (0777 | S_IFMT);
  // klock(&dir->lock_);
  klock(&ino->lock_);

  if (attach_inode(ino, dir, name)) {
    kunlock (&dir->lock_);
    kunlock (&ino->lock_);
    kfree(ino);
    return NULL;
  }

  klist_push_front(&kSYS.inodeLru_, &ino->lruNd_);
  kunlock (&dir->lock_);
  return ino;
}


// ---------------------------------------------------------------------------
/** Release an inode form the inode cache. */
int unregister_inode (kInode_t *ino)
{
  assert (kislocked (&ino->parent_->lock_));
  assert (kislocked (&ino->lock_));
  assert (ino->child_ == NULL);
  assert (ino->readers_ == 0);

  // @todo ino->lock_.flags |= LK_DELETED;
  if (ino->prev_ != NULL) {
    klock(&ino->prev_->lock_);
    ino->prev_->next_ = ino->next_;
    kunlock(&ino->prev_->lock_);
  } else {
    ino->parent_->child_ = ino->next_;
  }

  if (ino->next_ != NULL) {
    klock(&ino->next_->lock_);
    ino->next_->prev_ = ino->prev_;
    kunlock(&ino->next_->lock_);
  }

  kunlock (&ino->lock_);
  kunlock (&ino->parent_->lock_);
  klist_remove (&kSYS.inodeLru_, &ino->lruNd_);

  // @todo Free all buckets and stream objects...
  // @todo Push to garbadge candidate
  // @todo Free name
  if (ino->pagesCache_ != NULL) {
    kprintf (LOG, "We need to clean pages\n");
  }

  kfree((void *)ino->name_);
  kfree(ino);

  return __noerror ();
}


// ---------------------------------------------------------------------------
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes)
{
  while (nodes-- > 0) {

    kInode_t *ino;

    for (ino = klist_begin(&kSYS.inodeLru_, kInode_t, lruNd_);
         ino != NULL;
         ino = klist_next(ino, kInode_t, lruNd_)) {

      assert (ino->parent_ != NULL);
      klock (&ino->parent_->lock_);
      klock (&ino->lock_);

      // @todo -- check that we are not closing a mouting point
      if (ino->readers_ == 0 && ino->child_ == NULL) {
        int err = unregister_inode(ino);

        if (err)
          return err;

        break;
      }

      kunlock (&ino->lock_);
      kunlock (&ino->parent_->lock_);
    }

    if (ino == NULL)
      return __seterrno (EINVAL);
  }

  return __seterrno (0);
}


// ---------------------------------------------------------------------------
/** Call the bucket scavanger which will try to free cached page. */
int scavenge_bucket(int pages)
{
  return __seterrno (ENOSYS);
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
