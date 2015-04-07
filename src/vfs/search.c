/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to search a inodes from it's name and/or path.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"
#include "kernel/info.h"


// ---------------------------------------------------------------------------
/** Search an inode on a directory
  * @param[in]    name        The name of the file to search
  * @param[in]    dir         The directory to search
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
  assert (PARAM_FILENAME(name));
  assert (dir != NULL);
  assert (kislocked(&dir->lock_));

  // Loop over present children.
  kInode_t *ino = dir->child_;

  while (ino) {
    int cmp = strcmp(ino->name_, name);

    if (cmp == 0) {
      klock (&ino->lock_);
      kunlock (&dir->lock_);
      return ino;
    }

    ino = ino->next_;
  }

  // Test is file-system allow search.
  if (!dir->dev_->lookup) {
    kunlock (&dir->lock_);
    __seterrno (ENOENT);
    kprintf (LOG, "Can't find %s on dir %s\n", name, dir->name_);
    return NULL;
  }

  // Ask the driver for the file.
  kStat_t stat;
  MODULE_ENTER(&dir->lock_, &dir->dev_->lock_);
  int err = dir->dev_->lookup (name, dir, &stat);
  MODULE_LEAVE(&dir->lock_, &dir->dev_->lock_);

  if (err != 0) {
    kunlock (&dir->lock_);
    __seterrno (err == ENOENT ? ENOENT : EIO);
    kprintf (LOG, "Can't find %s on dir %s\n", name, dir->name_);
    return NULL;
  }

  ino = register_inode (name, dir, &stat);
  return ino;
}


// ---------------------------------------------------------------------------
/** Search an inode on the filetree.
  * @param[in]    path        The file descriptor
  * @param[in]    dir         The initial node where to start the search.
  * This function will split each directory of the path and look for it. The
  * search start a dir. If dir is NULL the routine will request the current
  * directory. In any case if the path start by '/'. The directory will be
  * replace by 'root'.
  * @retval      ENOTDIR         No write access to the parent directory.
  * @retval      -> follow_symlink().
  * @retval      -> search_child().
  */
kInode_t *search_inode (const char *path, kInode_t *dir)
{
  char uri [PATH_MAX];
  int symLinkLoop = 0;
  char *rentTok;

  assert (klockcount() == 0);
  __seterrno (0);

  // Find the initial directory.
  if (strrchr (path, ':')) {
    // @todo read volume
  } else if (path[0] == '/' || path[0] == '\\')
    dir = kSYS.rootNd_;
  else if (dir == NULL)
    dir = kSYS.rootNd_; // @todo replace by PWD

  strncpy (uri, path, PATH_MAX);
  char *name = NULL;
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
        assert (klockcount() == 0);
        return NULL;
      }
    }

    // Check if we are on a directory.
    if (!S_ISDIR (dir->stat_.mode_)) {
      kunlock (&dir->lock_);
      __seterrno(ENOTDIR);
      return NULL;
    }

    // Handle special names
    if (strcmp(".", name) == 0) {
      continue;

    } else if (strcmp("..", name) == 0) {
      kunlock (&dir->lock_);
      dir = dir->parent_;
      klock (&dir->lock_);

#ifdef DIR_THREE_DOT
    } else if (strcmp("...", name) == 0) {
      do {
        kunlock (&dir->lock_);
        dir = dir->parent_;
        klock (&dir->lock_);
      } while (dir->parent_ != NULL && !INO_IS_VOLUME(dir));

#endif

    } else {
      // Search child node
      dir = search_child (name, dir);

      if (dir == NULL) {
        assert (klockcount() == 0);
        return NULL;
      }
    }
  }

  // We read all folder on path, we found it.
  kunlock (&dir->lock_);
  assert (klockcount() == 0);
  return dir;
}


// ---------------------------------------------------------------------------
/** Give the inode a symbolic link is refering to. */
kInode_t *follow_symlink(kInode_t *ino, int *links)
{
  if (links != NULL)
    (*links)++;

  __seterrno(ENOSYS);
  return NULL;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
