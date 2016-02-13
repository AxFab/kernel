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
#include <smkos/core.h>
#include <smkos/mod/vfs.h>


/* Search an inode on a directory. */
static inode_t *search_child (const char *name, inode_t *dir)
{
  int ret;
  stat_t stat;
  inode_t *ino = dir->first_child_;

  assert(ck_name(name));
  assert(dir != NULL);
  assert(rd_locked(&dir->lock_));

  /* Loop over present children. */
  while (ino) {
    if (dir->dev_->drv_->case_sensitive_)
      ret = strcmp(ino->name_, name);
    else
      ret = strcmpi(ino->name_, name);

    if (ret == 0) {
      rd_lock (&ino->lock_);
      rd_unlock (&dir->lock_);
      return ino;
    }

    ino = ino->next_sibling_;
  }

  /* Call driver. */
  if (dir->dev_->drv_->lookup == NULL) {
    printf("Error: file system doesn't implement lookup method.\n");
    errno = ENOSYS;
    return NULL;
  } else if (driver_access(dir)) {
    return NULL;
  }
  
  // async_run(fs_job, ino->dev_->fs_->lookup, name, dir, &stat);
  ret = dir->dev_->drv_->lookup(name, dir, &stat);
  driver_release(dir);
  if (ret) {
    rd_unlock(&dir->lock_);
    return NULL;
  }
  
  return inode_register(name, dir, &stat);
}

/* Select the directory inode to start a search */
static inode_t *search_init(const char *path, inode_t *dir)
{
  if (strrchr (path, ':')) {
    /// @todo read volume
  } else if (path[0] == '/' || path[0] == '\\') {
    dir = __VFS.root_ino_;
  } else if (dir == NULL) {
    dir = __PWD;
    if (dir == NULL)
      dir = __VFS.root_ino_;
  }

  return dir;
}

/* Evaluate one element of a path during an inode search. */
static inode_t *search_iterate(const char *name, inode_t *dir, int flags, int *links)
{
  assert(rd_locked(&dir->lock_));

  /* Follow symlink. */
  if (S_ISLNK(dir->stat_.mode_)) {
    dir = symlink_follow(dir, links);
    if (dir == NULL)
      return NULL;
  }

  /* Check if we are on a directory. */
  if (!S_ISDIR (dir->stat_.mode_)) {
    rd_lock (&dir->lock_);
    errno = ENOTDIR;
    return NULL;
  }

  /* Handle special names */
  do {
    if (name[0] != '.')
      break;

    if (name[1] == '\0')
      return dir; /* `./' -> current dir */
    else if (name[1] != '.')
      break;

    if (name[2] == '\0') {
      rd_unlock(&dir->lock_);
      dir = dir->parent_;
      rd_lock(&dir->lock_);
      return dir; /* `../' -> parent dir */
    } else if (name[2] != '.')
      break;

    if ((flags & AT_THREE_DOT) && name[3] == '\0') {
      rd_unlock(&dir->lock_);
      do {
        dir = dir->parent_;
      } while (dir->parent_ != NULL && dir->dev_ == dir->parent_->dev_);

      rd_lock(&dir->lock_);
      return dir; /* `.../' -> parent volume */
    }
  } while(0);

  /* Search child node */
  dir = search_child(name, dir);
  return dir;
}

/* Search an inode on the filetree. */
inode_t *inode_lookup(const char *path, inode_t *dir, int flags, int *links, char* lfile)
{
  char *uri;
  char *name = NULL;
  char *lname;
  int symLinkLoop = 0;
  char *rentTok;
  int uriLg = strlen(path);
  inode_t *prev;
  

  if (links == NULL)
    links = &symLinkLoop;

  if (ck_path(path)) {
    errno = EINVAL;
    return NULL;
  }
  
  assert(irq_check_on());
  errno = 0;

  /* Find the initial directory. */
  dir = search_init(path, dir);
  uri = strdup(path);
  rd_lock(&dir->lock_);

  /* For each element of the pathname. */
  for (name = strtok_r(uri, "/\\", &rentTok);
       name != NULL;
       name = strtok_r(NULL, "/\\", &rentTok) ) {

    prev = dir;
    lname = name;
    dir = search_iterate(name, dir, flags, links);
    if (dir == NULL) {
      if (lfile && flags & AT_GET_PARENT && strtok_r(NULL, "/\\", &rentTok) == NULL) {
        strcpy(lfile, lname);
        free(uri);
        return prev;
      }
      
      free(uri);
      return NULL;
    }
  }

  /* We read all elements of the path, we found it. */
  rd_unlock(&dir->lock_);
  if (lfile) 
    strcpy(lfile, lname);
  free(uri);
  return (flags & AT_GET_PARENT) ? prev : dir;
}

/* Give the inode a symbolic link is refering to. */
inode_t *symlink_follow(inode_t *ino, int *links)
{
  int ret;
  inode_t *lnk;
  int lg = MIN(PATH_MAX, ino->stat_.length_ + 1);
  char* buf;
  if (links != NULL)
    (*links)++;

  errno = 0;
  assert(rd_locked(&ino->lock_));
  if (ino->s.symlink_ != NULL) {
    lnk = ino->s.symlink_;
    rd_unlock(&ino->lock_);
    rd_lock(&lnk->lock_);
    return lnk;
  }

  if (ino->dev_->drv_->readlink == NULL) {
    printf("Error: driver create a symlink but can't reading it.\n");
    errno = ENOSYS;
    return NULL;
  } else if (driver_access(ino)) {
    return NULL;
  }
  
  buf = (char*)malloc(lg);
  // async_run(fs_job, ino->dev_->fs_->readlink, ino, buf, lg);
  ret = ino->dev_->drv_->readlink(ino, buf, lg);
  driver_release(ino);
  rd_unlock(&ino->lock_);

  if (ret) {
    free(buf);
    return NULL;
  }

  lnk = inode_lookup(buf, ino->parent_, AT_SYMLINK, links, NULL);
  rd_lock(&lnk->lock_);
  atomic_inc(&lnk->usage_);
  ino->s.symlink_ = lnk;
  free(buf);
  return lnk;
}

/* Read the full path of the file: UNIX style */
int inode_readlink(inode_t* ino, char* buf, int lg)
{
  int ret;
  char *uri;
  buf[0] = '\0';
  
  errno = 0;
  if (!ino) { 
    errno = EINVAL;
    return -EINVAL;
  }

  uri = malloc(PATH_MAX);
  for (;;) {
    if (ino->parent_ == NULL) {
      free (uri);
      return 0;
    }

    ret = snprintf(uri, PATH_MAX, "/%s%s", ino->name_, buf);
    strncpy(buf, uri, lg);
    if (ret < 0 || ret > PATH_MAX) {
      kfree (uri);
      errno = ENAMETOOLONG;
      return -ENAMETOOLONG;
    }

    ino = ino->parent_;
  }
}

/* Read the full path of the file: NT style */
int inode_readuri(inode_t* ino, char* buf, int lg)
{
  int ret;
  char *uri;
  buf[0] = '\0';
  
  errno = 0;
  if (!ino) { 
    errno = EINVAL;
    return -EINVAL;
  }
  
  uri = malloc(PATH_MAX);
  for (;;) {
    if (ino == NULL || ino->parent_ == NULL) {
      ret = snprintf(uri, PATH_MAX, "%s:%s", "S", buf);

    } else if (ino->parent_ != NULL && ino->dev_ != ino->parent_->dev_) {
      ret = snprintf(uri, PATH_MAX, "%s:%s", ino->name_, buf);

    } else {
      ret = snprintf(uri, PATH_MAX, "\\%s%s", ino->name_, buf);
    }

    if (ret < 0 || ret > PATH_MAX) {
      free (uri);
      errno = ENAMETOOLONG;
      return -ENAMETOOLONG;
    }
    
    strncpy(buf, uri, lg);
    if (ino == NULL || ino->parent_ == NULL || ino->dev_ != ino->parent_->dev_) {
      free (uri);
      return 0;
    }

    ino = ino->parent_;
  }
}

