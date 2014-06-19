#include <inodes.h>

// ===========================================================================
/**
 * Attach a new inode to the file system hierarchie.
 *   EEXIST the method found an item with the same name
 * @note The inode are attach in alphabetic order.
 */
static int kFs_Attach (kInode_t* ino, kInode_t* top, const char* name)
{
  int k;
  kInode_t* cursor = top->child_;
  assert (kislocked (&top->lock_));
  assert (kislocked (&ino->lock_));
  ino->parent_ = top;

  if (top->child_ == NULL) {
    top->child_ = ino;
    return __noerror ();
  }

  if (0 < strcmp(cursor->name_, name)) {
    ino->next_ = top->child_;
    cursor->prev_ = ino;
    top->child_ = ino;
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

// ===========================================================================

kInode_t* kFs_LookFor(const char* path, kInode_t* dir)
{
  kStat_t stat;
  kInode_t* inode;
  int k, symLinkLoop = 0;
  char uri [PATH_MAX];
  char* name;
  char* rentTok;
  __noerror();
  // FIXME should lock
  strncpy (uri, path, PATH_MAX);
  name = strtok_r (uri, FILENAME_SEPARATOR, &rentTok);

  if (dir == NULL || uri [0] == '/') {
    dir = kFs_RootInode();
  }

  // We found it!
  if (name == NULL) {
    return dir;
  }

  // Follow symlink
  if ( S_ISLNK (dir->stat_.mode_) ) {
    kFs_FollowLink(&dir, &symLinkLoop);

    if (__geterrno()) {
      return NULL;
    }

    if (symLinkLoop > MAX_SYMLINK_LOOP) {
      __seterrno (ELOOP);
      return NULL;
    }
  }

  // Is a directory
  if ( !S_ISDIR (dir->stat_.mode_)) {
    __seterrno(ENOTDIR);
    return NULL;
  }

  if (dir->child_ == NULL) {
    // kprintf ("Inode search find empty dir %s, look for %s \n", top->name_, name);
    if (!dir->fs_->lookup) {
      __seterrno (ENOENT);
      return NULL;
    }

    memset (&stat, 0, sizeof(kStat_t));

    // FIXME Call driver
    if (dir->fs_->lookup (name, dir, &stat)) {
      __seterrno (ENOENT);
      return NULL;
    }

    inode = kFs_Register (name, dir, &stat);
    kunlock (&inode->lock_);
  }

  dir = dir->child_;
  // END FIRST

  for (;;) {
    k = strcmp(dir->name_, name);

    if (k != 0 && dir->next_ != NULL) {
      dir = dir->next_;
      continue;

    } else if (k != 0) {
      if (!dir->parent_->fs_->lookup) {
        __seterrno (ENOENT);
        return NULL;
      }

      memset (&stat, 0, sizeof(kStat_t));

      // FIXME Call driver
      if (dir->parent_->fs_->lookup (name, dir->parent_, &stat)) {
        __seterrno (ENOENT);
        return NULL;
      }

      inode = kFs_Register (name, dir->parent_, &stat);
      kunlock (&inode->lock_);
      dir = dir->parent_->child_;
      continue;
    }

    // Check access permissions
    // kSecurity()
    name = strtok_r (NULL, FILENAME_SEPARATOR, &rentTok);
    // kprintf ("Inode search iteration on %s, look for %s \n", top->name_, name);

    if (name == NULL) { // We found it!
      return dir;
    }

    // Follow symlink
    if ( S_ISLNK (dir->stat_.mode_) ) {
      kFs_FollowLink(&dir, &symLinkLoop);

      if (__geterrno()) {
        __geterrno();
        return NULL;
      }

      if (symLinkLoop > MAX_SYMLINK_LOOP) {
        __seterrno (ELOOP);
        return NULL;
      }
    }

    // Is a directory
    if ( !S_ISDIR (dir->stat_.mode_)) {
      __seterrno(ENOTDIR);
      return NULL;
    }

    if (dir->child_ == NULL) {
      // kprintf ("Inode search find empty dir %s, look for %s \n", top->name_, name);
      if (!dir->fs_->lookup) {
        __seterrno (ENOENT);
        return NULL;
      }

      memset (&stat, 0, sizeof(kStat_t));

      // FIXME Call driver
      if (dir->fs_->lookup (name, dir, &stat)) {
        __seterrno (ENOENT);
        return NULL;
      }

      inode = kFs_Register (name, dir, &stat);
      kunlock (&inode->lock_);
    }

    dir = dir->child_;
  }
}

// ===========================================================================

kInode_t* kFs_Register(const char* name, kInode_t* dir, kStat_t* stat)
{
  kInode_t* ino;
  __noerror ();

  if (name == NULL || stat == NULL) {
    __seterrno (EINVAL);
    return NULL;
  }

  if (dir == NULL) dir = kFs_RootInode();

  if ( !S_ISDIR (dir->stat_.mode_)) {
    __seterrno(ENOTDIR);
    return NULL;
  }

  ino = (kInode_t*) kalloc(sizeof(kInode_t));
  ino->name_ = kcopystr(name);
  ino->fs_ = dir->fs_; // FIXME Not always !?
  memcpy (&ino->stat_, stat, sizeof(kStat_t));
  ino->stat_.no_ = (ino_t)ino;
  ino->stat_.mode_ &= (S_IALLUGO | S_IFMT);
  klock(&dir->lock_);
  klock(&ino->lock_);

  if (kFs_Attach (ino, dir, name)) {
    kunlock (&dir->lock_);
    kunlock (&ino->lock_);
    kfree(ino);
    return NULL;
  }

  kunlock (&dir->lock_);
  return ino;
}

// ===========================================================================

int kFs_Unregister(kInode_t* ino)
{
  return __seterrno (ENOSYS);
}


