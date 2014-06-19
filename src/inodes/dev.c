#include <inodes.h>


kInode_t* kFs_Mount (const char* name, kInode_t* dir, kInode_t* dev, int (*fsEntry)(kFsys_t* fs, kStat_t* file))
{
  int err;
  kStat_t file;
  kFsys_t* fs = KALLOC(kFsys_t);
  __noerror();

  if (dev)
    fs->dev_ = dev->stat_.dev_;

  memset (&file, 0, sizeof(kStat_t));
  err = fsEntry (fs, &file);

  if (err) {
    kfree (fs);
    kunlock (&dir->lock_);
    __seterrno (EBUSY);
    return NULL;
  }

  kInode_t* ino = kFs_MkNode(name, dir, &file);

  if (ino == NULL) {
    kfree (fs);
  }

  ino->fs_ = fs;
  return ino;
}


kInode_t* kFs_CreateBlock (const char* name, kInode_t* dir, dev_t fd, int (*driverEntry)(kDevice_t* dev))
{
  int err;
  time_t now = ktime();
  kDevice_t* drive = KALLOC(kDevice_t);
  __noerror();
  drive->fd_ = fd;
  err = driverEntry (drive);

  if (err) {
    kfree (drive);
    __seterrno (err);
    return NULL;
  }

  kStat_t rBlk = {
    0, S_IFBLK | 0700, ROOT_UID, ROOT_UID,
    drive->size_, 0, now, now, now, drive
  };
  kInode_t* ino = kFs_MkNode(name, dir, &rBlk);

  if (ino == NULL) {
    kfree (drive);
  }

  return ino;
}


