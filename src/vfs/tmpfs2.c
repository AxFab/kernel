#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>


/* ----------------------------------------------------------------------- */
int TMPFS_mount(kInode_t *dev, const char *name)
{
  if (dev != NULL)
    return ENOSYS;

  return EIO;
}

/* ----------------------------------------------------------------------- */
int TMPFS_create(const char *name, kInode_t *dir, int mode, size_t lg, SMK_stat_t *stat)
{
  time_t now = time(NULL);

  memset(stat, 0, sizeof(*stat));
  stat->atime_ = now;
  stat->ctime_ = now;
  stat->mtime_ = now;
  stat->block_ = PAGE_SIZE;
  stat->mode_ = mode;
  stat->length_ = lg;

  switch (mode & S_IFMT) {
  case S_IFREG:
  case S_IFDIR:
  case S_IFIFO:
  case S_IFLNK:
  case S_IFTTY:
    return 0;

  default:
    return EINVAL;
  }
}

/* ----------------------------------------------------------------------- */
int TMPFS_symlink(const char *name, kInode_t *dir, int mode, const char *path, SMK_stat_t *stat)
{
  int ret;
  int lg = strlen(path) + 1;
  ret = TMPFS_create(name, dir, mode, lg, stat);
  stat->lba_ = (size_t)strdup(path);
  stat->length_ = lg;
  return ret;
}

/* ----------------------------------------------------------------------- */
int TMPFS_readlink(kInode_t *fp, char* path, int lg)
{
  assert (fp->stat_.length_ < PATH_MAX);

  if (S_ISLNK(fp->stat_.mode_)) {
    strncpy(path, (char*)fp->stat_.lba_, MAX(lg, (int)fp->stat_.length_));
    return 0;
  }

  return ENOSYS;
}

int TMPFS_release(kInode_t *fp)
{
  if (S_ISLNK(fp->stat_.mode_)) {
    kfree((char*)fp->stat_.lba_);
  }

  return 0;
}

void TMPFS (kDriver_t *driver)
{
  driver->name_ = strdup("tmpfs");
  driver->mount = TMPFS_mount;
  driver->create = TMPFS_create;
  driver->symlink = TMPFS_symlink;
  driver->readlink = TMPFS_readlink;
  driver->release = TMPFS_release;
}