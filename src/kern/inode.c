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
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>

/* INODES
  * The inode module manage files entry.
  *
  * The kernel keep on cache the entry available on various device mounted on
  * the system.
  */


/* ----------------------------------------------------------------------- */
/** @brief Search an inode on a directory.
  * @param name   [in] The name of the file to search
  * @param dir    [in] The directory to search
  * This function will browse the child of a directory and will request the
  * file system if needed.
  * @retval      EIO             An I/O error occurred.
  * @retval      ENOENT          An element of path doesn't exist.
  * @retval      -> register_inode().
  * @bug Think about case-sensitive file systems.
  * @note The directory inode must be locked, and parameters are not checked.
  */
static kInode_t *search_child (const char *name, kInode_t *dir)
{
  int err;
  SMK_stat_t stat;
  kInode_t *ino = dir->child_;

  assert (name != NULL && strnlen(name, FNAME_MAX) < FNAME_MAX);
  assert (dir != NULL);
  assert (kislocked(&dir->lock_));

  // Loop over present children.
  while (ino) {
    int cmp = strcmp(ino->name_, name);

    if (cmp == 0) {
      klock (&ino->lock_);
      kunlock (&dir->lock_);
      return ino;
    }

    ino = ino->next_;
  }

  // Search on file system.
  if (!dir->dev_->fs_->lookup) {
    kunlock(&dir->lock_);
    return NULL;
  }

  if (open_fs(dir))
    return NULL;

  err = dir->dev_->fs_->lookup(name, dir, &stat);
  close_fs(dir);
  if (err != 0)
    return NULL;

  ino = register_inode (name, dir, &stat, false);
  return ino;
}


/* ----------------------------------------------------------------------- */
/** @brief Search an inode on the filetree.
  * @param path   [in] The file descriptor
  * @param dir    [in] The initial node where to start the search.
  * @param flags  [in] Flag to use to manipulate the search (cf. AT_SEARCH).
  * This function will split each directory of the path and look for it. The
  * search start a dir. If dir is NULL the routine will request the current
  * directory. In any case if the path start by '/'. The directory will be
  * replace by 'root'.
  * @retval      ENOTDIR         No write access to the parent directory.
  * @retval      -> follow_symlink().
  * @retval      -> search_child().
  */
kInode_t *search_inode (const char *path, kInode_t *dir, int flags)
{
  char *uri;
  char *name = NULL;
  int symLinkLoop = 0;
  char *rentTok;
  int uriLg = strlen(path);

  if (uriLg > PATH_MAX)
    return __seterrnoN(EINVAL, kInode_t);

  uri = kalloc(uriLg + 1);

  assert (kCPU.lockCounter_ == 0);
  __seterrno (0);

  // Find the initial directory.
  if (strrchr (path, ':')) {
    /// @todo read volume
  } else if (path[0] == '/' || path[0] == '\\')
    dir = kSYS.rootIno_;
  else if (dir == NULL)
    dir = kSYS.rootIno_; /// @todo replace by PWD

  strncpy (uri, path, uriLg);
  klock (&dir->lock_);

  // For each folder inside the pathname.
  for (name = strtok_r (uri, "/\\", &rentTok);
       name != NULL;
       name = strtok_r (NULL, "/\\", &rentTok) ) {

    assert (kislocked(&dir->lock_));

    // Follow symlink.
    if (S_ISLNK(dir->stat_.mode_)) {
      dir = follow_symlink(dir, &symLinkLoop);

      if (dir == NULL) {
        kfree(uri);
        return NULL;
      }
    }

    // Check if we are on a directory.
    if (!S_ISDIR (dir->stat_.mode_)) {
      kunlock (&dir->lock_);
      kfree(uri);
      return __seterrnoN(ENOTDIR, kInode_t);
    }

    // Handle special names
    if (name[0] == '.') {
      if (name[1] == '\0')
        continue;

      if (name[1] == '.') {
        if (name[1] == '\0') {
          kunlock (&dir->lock_);
          dir = dir->parent_;
          klock (&dir->lock_);
          continue;
        }

        if ((flags & AT_THREE_DOT) && name[2] == '.' && name[3] == '\0') {
          kunlock (&dir->lock_);
          do {
            dir = dir->parent_;
          } while (dir->parent_ != NULL/* && !S_ISVOL(dir->stat_.mode_)*/);
          klock (&dir->lock_);
          continue;
        }
      }
    }

    // Search child node
    dir = search_child (name, dir);
    if (dir == NULL) {
      kfree(uri);
      return NULL;
    }
  }

  // We read all folder on path, we found it.
  kunlock (&dir->lock_);
  kfree(uri);
  return dir;
}


/* ----------------------------------------------------------------------- */
/** @brief Attach an inode to its parent. */
static int attach_inode (kInode_t *ino, kInode_t *dir, const char *name)
{
  int k;
  kInode_t *cursor = dir->child_;

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
kInode_t *register_inode (const char *name, kInode_t *dir, SMK_stat_t *stat, bool unlock)
{
  kInode_t *ino;
  assert (name != NULL && strnlen(name, FNAME_MAX) < FNAME_MAX);
  assert (dir != NULL && stat != NULL);
  assert (S_ISDIR (dir->stat_.mode_));

  klock(&dir->lock_);
  ino = KALLOC(kInode_t);
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
int unregister_inode (kInode_t *ino)
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
  kunlock (&ino->parent_->lock_);

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
kInode_t *create_inode(const char* name, kInode_t* dir, int mode, size_t lg)
{
  int err;
  SMK_stat_t stat;

  assert (name != NULL);
  assert (dir != NULL);

  if (strnlen(name, FNAME_MAX) >= FNAME_MAX)
    return __seterrnoN(ENAMETOOLONG, kInode_t);

  klock(&dir->lock_);
  if (open_fs(dir))
    return NULL;

  err = dir->dev_->fs_->create(name, dir, mode, lg, &stat);
  close_fs(dir);

  if (err)
    return __seterrnoN(err, kInode_t);

  return register_inode(name, dir, &stat, true);
}


/* ----------------------------------------------------------------------- */
/** @brief Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes)
{
  int err;
  int deleted;
  kInode_t *ino;
  kInode_t *iter;

  while (nodes-- > 0) {

    iter = ll_first(&kSYS.inodeLru_, kInode_t, lruNd_);
    if (iter == NULL)
      return __seterrno (EINVAL);

    deleted = 0;
    while (iter)
    {
      ino = iter;
      iter = ll_next(iter, kInode_t, lruNd_);

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
int inode_open (kInode_t *ino)
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
int inode_close (kInode_t *ino)
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
/** @brief Give the inode a symbolic link is refering to. */
kInode_t *follow_symlink(kInode_t *ino, int *links)
{
  if (links != NULL)
    (*links)++;

  __seterrno(ENOSYS);
  return NULL;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
