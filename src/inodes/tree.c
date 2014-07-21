#include <inodes.h>
#include <kinfo.h>

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

static kInode_t* kFs_LookChild (const char* name, kInode_t* dir)
{
  int err;
  kStat_t stat;

  if (!dir->fs_->lookup) {
    __seterrno (ENOENT);
    return NULL;
  }

  // Request the file system
  MOD_ENTER;
  memset (&stat, 0, sizeof(kStat_t));
  err = dir->fs_->lookup (name, dir, &stat);
  MOD_LEAVE;
  if (err) {
    __seterrno (err);
    return NULL;
  }

  return kFs_Register (name, dir, &stat);
}

// ----------------------------------------------------------------------------

kInode_t* kFs_LookFor(const char* path, kInode_t* dir)
{
  int k;
  int symLinkLoop = 0;
  char uri [PATH_MAX];
  char* name;
  char* rentTok;
  kInode_t* ino;

  NO_LOCK;

  // IF VOLUME LOOK FOR VOLUME
  if (strrchr (path, VOLUME_SEPARATOR)) {
    // FIXME read volume
  }

  // IF NO DIR OR / START WITH ROOT
  if (dir == NULL || path[0] == '/' || path[0] == '\\')
    dir = kFs_RootInode();

  // Look the first node file
  strncpy (uri, path, PATH_MAX);
  name = strtok_r (uri, FILENAME_SEPARATORS, &rentTok);
  klock (&dir->lock_, LOCK_FS_LOOK);
  while (name != NULL) {

    // Follow symlink
    if (S_ISLNK(dir->stat_.mode_)) {
      kFs_FollowLink(&dir, &symLinkLoop);
      if (__geterrno()) {
        return NULL;
      }

      if (symLinkLoop > MAX_SYMLINK_LOOP) {
        kunlock (&dir->lock_);
        __seterrno (ELOOP);
        return NULL;
      }
    }

    // Is a directory
    if (!S_ISDIR (dir->stat_.mode_)) {
      kunlock (&dir->lock_);
      __seterrno(ENOTDIR);
      return NULL;
    }

    // If no child, request the file system
    if (dir->child_ == NULL) {
      kunlock (&dir->lock_);
      dir = kFs_LookChild (name, dir);
      if (dir == NULL)
        return NULL;

    } else {
      ino = dir->child_;
      for (;;) {

        k = strcmp(ino->name_, name);
        if (k != 0) {
          if (ino->next_ != NULL) {
            ino = ino->next_;
            continue;

          } else {
            kunlock (&dir->lock_);
            dir = kFs_LookChild (name, dir);
            if (dir == NULL)
              return NULL;

          }
        }

        klock (&ino->lock_, LOCK_FS_LOOK);
        kunlock (&dir->lock_);
        dir = ino;
        break;
      }
    }

    name = strtok_r (NULL, FILENAME_SEPARATORS, &rentTok);
  }

  kunlock (&dir->lock_);
  NO_LOCK;
  return dir;
}


// ===========================================================================
/** Try to add a new inode on the VFS tree 
 * The returned inode is still locked
 */
kInode_t* kFs_Register(const char* name, kInode_t* dir, kStat_t* stat)
{
  kInode_t* ino;
  __noerror ();

  if (name == NULL || dir == NULL || stat == NULL) {
    __seterrno (EINVAL);
    return NULL;
  }

  if ( !S_ISDIR (dir->stat_.mode_)) {
    __seterrno(ENOTDIR);
    return NULL;
  }

  ino = (kInode_t*) kalloc(sizeof(kInode_t));
  ino->name_ = kcopystr(name);
  ino->fs_ = dir->fs_;
  ino->devinfo_ = dir->devinfo_;
  memcpy (&ino->stat_, stat, sizeof(kStat_t));
  ino->stat_.ino_ = kSys_NewIno();
  ino->stat_.mode_ &= (S_IALLUGO | S_IFMT);
  klock(&dir->lock_, LOCK_FS_REGISTER);
  klock(&ino->lock_, LOCK_FS_REGISTER);
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


