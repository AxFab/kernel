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
 *      File and device entries managment.
 */
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <smkos/check.h>

#include "vfs.h"


/* ----------------------------------------------------------------------- */
/** @brief Attach an inode to its parent. */
static int attach_inode (inode_t *ino, inode_t *dir, const char *name)
{
  int k;
  inode_t *cursor = dir->child_;

  assert (kislocked (&dir->lock_));
  assert (kislocked (&ino->lock_));
  ino->parent_ = dir;

  if (dir->child_ == NULL) {
    dir->child_ = ino;
    return __seterrno(0);
  }

  if (0 < strcmp(cursor->name_, name)) {
    ino->next_ = dir->child_;
    cursor->prev_ = ino;
    dir->child_ = ino;
    return __seterrno(0);
  }

  for (;;) {
    if (cursor->next_ == NULL) {
      ino->prev_ = cursor;
      cursor->next_ = ino;
      return __seterrno(0);
    }

    k = strcmp(cursor->next_->name_, name);

    if (k < 0 && cursor->next_ != NULL) {
      cursor = cursor->next_;
      continue;
    }

    if (k == 0)
      return __seterrno(EEXIST);

    ino->prev_ = cursor;
    ino->next_ = cursor->next_;
    cursor->next_->prev_ = ino;
    cursor->next_ = ino;
    return __seterrno(0);
  }
}


/* ----------------------------------------------------------------------- */
/** @brief Try to add a new inode on the VFS tree. */
inode_t *register_inode (const char *name, inode_t *dir, SMK_stat_t *stat, bool unlock)
{
  inode_t *ino;
  assert (name != NULL && strnlen(name, FNAME_MAX) < FNAME_MAX);
  assert (dir != NULL && stat != NULL);
  assert (S_ISDIR (dir->stat_.mode_));

  klock(&dir->lock_);
  ino = KALLOC(inode_t);

  if (ino == NULL)
    return NULL;

  ino->name_ = strdup(name);
  ino->dev_ = dir->dev_;
  memcpy (&ino->stat_, stat, sizeof(SMK_stat_t));
  ino->dev_ = dir->dev_;
  ino->stat_.mode_ &= (0777 | S_IFMT);
  // klock(&dir->lock_);
  klock(&ino->lock_);

  if (attach_inode(ino, dir, name)) {
    kunlock (&dir->lock_);

    if (unlock)
      kunlock (&ino->lock_);

    kfree(ino);
    return NULL;
  }

  // Mounted point can't be pushed on LRU.
  if (ino->stat_.major_ == ino->parent_->stat_.major_ &&
      ino->stat_.minor_ == ino->parent_->stat_.minor_)
    ll_push_front(&kSYS.inodeLru_, &ino->lruNd_);

  kunlock (&dir->lock_);

  if (unlock)
    kunlock (&ino->lock_);

  return ino;
}


/* ----------------------------------------------------------------------- */
/** @brief Release an inode from the inode cache. */
int unregister_inode (inode_t *ino)
{
  kPage_t *page;

  assert (ino->parent_);
  assert (kislocked (&ino->parent_->lock_));
  assert (kislocked (&ino->lock_));
  // assert (ino->dev_ == ino->parent_->dev_);
  assert (ino->child_ == NULL);
  assert (ino->readers_ == 0);

  /// @todo ino->lock_.flags |= LK_DELETED;
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

  if (ino->dev_ == ino->parent_->dev_)
    ll_remove(&kSYS.inodeLru_, &ino->lruNd_);

  /* Free all pages */
  for (;;) {
    page = bb_best(&ino->pageTree_, kPage_t, treeNd_);

    if (!page)
      break;

    mmu_releasepage(page->phys_);
    bb_remove(&ino->pageTree_, &page->treeNd_);
    kfree(page);
  }

  kunlock (&ino->lock_);
  if (ino->dev_->fs_->release) {
    ino->prev_ = NULL;
    ino->next_ = NULL;
    if (open_fs(ino->parent_))
      kpanic("Memory leak, can free all");
    ino->parent_->dev_->fs_->release (ino);
    close_fs(ino->parent_);

  } else {
    kunlock (&ino->parent_->lock_);
  }

  /// @todo Free all buckets and stream objects...
  /// @todo Push to garbadge candidate
  /// @todo Free name
  /// @todo Free page cache first
  // if (ino->pagesCache_ != NULL) {
  //  kprintf ("We need to clean pages\n");
  // }

  kfree((void *)ino->name_);
  kfree(ino);

  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** @brief Request the file system for the creation of a new inode. */
inode_t *create_inode(const char *name, inode_t *dir, int mode, size_t lg)
{
  int err;
  SMK_stat_t stat;

  assert(name != NULL);
  assert(dir != NULL);
  assert(strchr(name, '/') == NULL);
  assert(strchr(name, '\\') == NULL);
  assert(strchr(name, ':') == NULL);

  if (strnlen(name, FNAME_MAX) >= FNAME_MAX)
    return __seterrnoN(ENAMETOOLONG, inode_t);


  if (dir->dev_->fs_->create == NULL) {
    return __seterrnoN(EROFS, inode_t);
  }

  klock(&dir->lock_);
  if (open_fs(dir))
    return NULL;

  err = dir->dev_->fs_->create(name, dir, mode, lg, &stat);
  close_fs(dir);

  if (err)
    return __seterrnoN(err, inode_t);

  return register_inode(name, dir, &stat, true);
}

/* ----------------------------------------------------------------------- */
inode_t* create_symlink(const char *name, inode_t *dir, const char* path)
{
  int err;
  SMK_stat_t stat;

  assert(name != NULL);
  assert(path != NULL);
  assert(dir != NULL);
  assert(strchr(name, '/') == NULL);
  assert(strchr(name, '\\') == NULL);
  assert(strchr(name, ':') == NULL);

  if (strnlen(name, FNAME_MAX) >= FNAME_MAX)
    return __seterrnoN(ENAMETOOLONG, inode_t);

  if (strnlen(path, PATH_MAX) >= PATH_MAX)
    return __seterrnoN(ENAMETOOLONG, inode_t);


  if (dir->dev_->fs_->symlink == NULL) {
    return __seterrnoN(EROFS, inode_t);
  }

  klock(&dir->lock_);
  if (open_fs(dir))
    return NULL;

  err = dir->dev_->fs_->symlink(name, dir, S_IFLNK | 0755, path, &stat);
  close_fs(dir);

  if (err)
    return __seterrnoN(err, inode_t);

  return register_inode(name, dir, &stat, true);
}

/* ----------------------------------------------------------------------- */
/** @brief Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes)
{
  int err;
  int deleted;
  inode_t *ino;
  inode_t *iter;

  while (nodes-- > 0) {

    iter = ll_first(&kSYS.inodeLru_, inode_t, lruNd_);

    if (iter == NULL)
      return __seterrno (EINVAL);

    deleted = 0;

    while (iter) {
      ino = iter;
      iter = ll_next(iter, inode_t, lruNd_);

      assert (ino->parent_ != NULL);
      klock (&ino->parent_->lock_);
      klock (&ino->lock_);

      /// @todo -- check that we are not closing a mouting point
      if (ino->readers_ == 0 && ino->child_ == NULL) {
        if (S_ISFIFO(ino->stat_.mode_) && ino->pipe_)
          fs_pipe_destroy(ino);
        else if (ino->assembly_ != NULL)
          destroy_assembly(ino->assembly_);

        err = unregister_inode(ino);

        if (err)
          return err;

        deleted++;
      } else {
        kunlock (&ino->lock_);
        kunlock (&ino->parent_->lock_);
      }
    }

    if (deleted == 0)
      return __seterrno (0);
  }

  return __seterrno (0);
}


/* ----------------------------------------------------------------------- */
/** @brief Function to called to grab an inodes */
int inode_open (inode_t *ino)
{
  /// @todo be sure that the file is available
  assert(ino != NULL);
  klock (&ino->lock_);
  ++ino->readers_;
  ll_remove_if(&kSYS.inodeLru_, &ino->lruNd_);
  kunlock (&ino->lock_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** @brief Function to release an inodes */
int inode_close (inode_t *ino)
{
  /// @todo if zero, push pression on scavenger
  assert(ino != NULL);
  klock (&ino->lock_);
  assert (ino->readers_ > 0);

  if (--ino->readers_ <= 0) {
    ll_remove_if(&kSYS.inodeLru_, &ino->lruNd_);

    // Mounted point can't be pushed on LRU.
    if (ino->dev_ == ino->parent_->dev_)
      ll_push_front(&kSYS.inodeLru_, &ino->lruNd_);
  }

  kunlock (&ino->lock_);
  return __seterrno(0);
}



/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
