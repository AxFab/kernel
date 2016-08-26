#include <smkos/vfs.h>
#include <skc/stat.h>


extern dirent_t *tsk_pwd();

static dirent_t *search_init(const TCHAR *path, dirent_t *ref);
static dirent_t *search_pathpart(const TCHAR *name, dirent_t *dir, int flags, int* lks);
static dirent_t *search_lookup(const TCHAR *path, dirent_t *ref, int flags, int* lks);
static dirent_t *search_symlink(dirent_t *ent, int flags, int* lks);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Find the start of a search */
static dirent_t *search_init(const TCHAR *path, dirent_t *ref)
{
  TCHAR *vol = tcschr(path, ':');
  if (vol != NULL) {
    // TODO -- read volume
    return NULL;
  } else if (vol[0] == '/' || vol[0] == '\\') {
    return __VFS.root_;
  } else if (ref != NULL) {
    return ref;
  } else {
    ref = tsk_pwd();
    if (ref == NULL) {
      return ref;
    } else {
      return __VFS.root_;
    }
  }
}

/* Search for the next part of path. */
static dirent_t *search_pathpart(const TCHAR *name, dirent_t *dir, int flags, int* lks)
{
  if (S_ISLNK(dir->ino_->mode_)) {
    dir = search_symlink(dir, flags, lks);
    if (dir == NULL)
      return NULL;
  }

  if (!S_ISDIR(dir->ino_->mode_)) {
    rw_rdlock(&dir->lock_);
    errno = ENOTDIR;
    return NULL;
  }

  if (name[0] == '.') {
    if (name[1] == '\0')
      return dir;
    if (name[1] == '.') {
      if (name[2] == '\0') {
        rw_rdunlock(&dir->lock_);
        dir = dir->parent_;
        rw_rdlock(&dir->lock_);
        return dir;
      }
      if (flags & AT_THREE_DOT && name[2] == '.' && name[3] == '\0') {
        rw_rdunlock(&dir->lock_);
        do {
          dir = dir->parent_;
        } while(dir->parent_ != NULL && dir->dev_ == dir->parent_->dev_);
        rw_rdlock(&dir->lock_);
        return dir;
      }
    }
  }

  dir = search_child(name, dir);
  return dir;
}

/* Search an inode on the file-tree */
static dirent_t *search_lookup(const TCHAR *path, dirent_t *ref, int flags, int* lks)
{
  TCHAR *uri = tcsdup(path);
  TCHAR *name, *rent;

  ref = search_init(path, ref);
  if (ref == NULL)
    return NULL;
  rw_rdlock(&ref->lock_);
  for (name = tcstok_r(uri, "/\\", &rent);
      name != NULL;
      name = tcstok_r(NULL, "/\\", &rent)) {
    ref = search_pathpart(name, ref, flags, lks);
    if (ref == NULL) {
      kfree(uri);
      return NULL;
    }
  }

  kfree(uri);
  return ref; 
}

/* Follow a symlink */
static dirent_t *search_symlink(dirent_t *ent, int flags, int* lks) 
{
  int err;
  int lg = MIN(PATH_MAX, ent->ino_->length_ + 1);
  dirent_t *link;
  TCHAR *buf;

  ++(*lks);
  if (ent->slink_ != NULL) {
    link = ent->slink_;
    rw_rdunlock(&ent->lock_);
    rw_rdlock(&link->lock_);
    return link;
  } 

  buf = (TCHAR*)kalloc(lg);
  err = drvfs_readlink(ent->ino_, buf, lg);
  if (err) {
    kfree(buf);
    return NULL;
  }

  link = search_lookup(buf, ent->parent_, flags, lks);
  ent->slink_ = link;
  atomic_inc(&link->readers_);
  rw_rdunlock(&ent->lock_);
  kfree(buf);
  return link;
}

/* Search for the child of a node */
dirent_t *search_child(const TCHAR *name, dirent_t *dir)
{
  int cmp;
  int err;
  driver_t *drv = dir->dev_->drv_;
  dirent_t *ent = dir->child_;

  /* Loop over present children */  
  while (ent) {
    cmp = drv->compare(ent->name_, name);
    if (cmp == 0) {
      rw_rdlock(&dir->lock_);
      return ent;
    }

    ent = ent->next_;
  }

  /* Search on file system */
  return drvfs_lookup(name, dir);
}

/* Search an inode on the file-tree */
dirent_t *vfs_lookup(const TCHAR *path, dirent_t *ref, int flags)
{
  int links;
  dirent_t *ent = search_lookup(path, ref, flags, &links);
  atomic_inc(&ent->readers_);
  sp_lock(&__VFS.lock_);
  ll_remove_if(&__VFS.lru_, &ent->lruNd_);
  sp_unlock(&__VFS.lock_);
  rw_rdunlock(&ent->lock_);
  return ent;
}

/* Dereference an inode return by `lookup' function */
void vfs_release(dirent_t *ent)
{
  rw_rdlock(&ent->lock_);
  if (atomic_xadd(&ent->readers_, -1) <= 1) {
    if (ent->dev_ == ent->parent_->dev_)
      ll_push_back(&__VFS.lru_, &ent->lruNd_);
  }
  rw_rdunlock(&ent->lock_);
  // TODO -- if low on memory add pressure on scavenger
}

