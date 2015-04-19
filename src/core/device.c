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
 *      Device support and driver managment.
 */
#include <smkos/core.h>
#include <smkos/_drv.h>


/* ----------------------------------------------------------------------- */
static int unregister_driver ()
{
  return __seterrno(ENOSYS);
}


/* ----------------------------------------------------------------------- */
static kDriver_t *search_driver(int major)
{
  kDriver_t *driver;
  ll_for_each(&kSYS.driverPool_, driver, kDriver_t, allNd_) {
    if( driver->major_ == major)
      return driver;
  }

  return NULL;
}


/* ----------------------------------------------------------------------- */
static void display_inode(kInode_t* ino, int depth)
{
  int i, j;
  kInode_t* dir;

  const char ftype[] = {
    'a', '-', 'd', '3', 'b', 'f', 'g', 'h', 'v', 'j', };
  const char* frights[] = {
    "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx", "???", };

  while (ino) {

    kprintf("%c%s%s%s %2d ",
            ftype[ino->stat_.mode_ >> 16],
            frights[(ino->stat_.mode_ >> 6) & 07],
            frights[(ino->stat_.mode_ >> 3) & 07],
            frights[(ino->stat_.mode_ >> 0) & 07],
            ino->readers_);
    kprintf("  %#03x <%2d, %2d> %4d  %s ",
            ino->stat_.uid_,
            ino->stat_.major_,
            ino->stat_.minor_,
            ino->stat_.block_,
            kpsize(ino->stat_.length_));

    for (i = 0; i < depth-1; ++i) {
      j = i;
      dir = ino;
      while (j++ < depth-1)
        dir = dir->parent_;
      if (dir->next_)
        kprintf("    |");
      else
        kprintf("     ");
    }

    if (depth > 0) {
      if (ino->next_)
        kprintf("    |--");
      else
        kprintf("    `--");
    }


    kprintf(" %s\n", ino->name_);

    if (ino->child_)
      display_inode(ino->child_, depth+1);

    ino = ino->next_;
  }
}


/* ----------------------------------------------------------------------- */
int open_fs(kInode_t* ino)
{
  assert (kislocked(&ino->lock_));
  assert (kCPU.lockCounter_ == 1);

  mtx_lock(&ino->dev_->mutex_);
  kunlock(&ino->lock_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
int close_fs(kInode_t* ino)
{
  // assert (mtx_grabed(&ino->dev_->mutex_));
  assert (kCPU.lockCounter_ == 0);

  mtx_unlock(&ino->dev_->mutex_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
void display_inodes()
{
  // kprintf(" ACCESS   | x | OWN | DEV  | BLOK | NAME \n");
  display_inode(kSYS.rootIno_, 0);
}


/* ----------------------------------------------------------------------- */
int mount_device(const char* name, kDevice_t* dev, kDriver_t* fs)
{
  kDriver_t *driver;
  klock(&dev->ino_->lock_);
  if (open_fs(dev->ino_))
    return __geterrno();
  ll_for_each(&kSYS.driverPool_, driver, kDriver_t, allNd_) {
    if (driver->mount (dev->ino_, name) == 0) {
      close_fs(dev->ino_);
      return 0;
    }
  }

  close_fs(dev->ino_);
  return __seterrno(ENOSYS);
}


/* ----------------------------------------------------------------------- */
void mount_alls ()
{
  kDevice_t* dev;
  ll_for_each (&kSYS.deviceList_, dev, kDevice_t, allNd_) {
    if (dev->usage_ == 0) {

      mount_device(NULL, dev, NULL);
    }
  }
}

// int ATAPI_Read (void *dr, uint32_t lba,  uint8_t sects, uint8_t *buf);
/* ----------------------------------------------------------------------- */
void initialize_vfs()
{
  kInode_t *root;
  time_t now = time(NULL);
  // char buf[2048];

  kprintf (KLOG_TRACE "Initializing virtual file system...\n");

  root = KALLOC(kInode_t);
  root->stat_.mode_ = S_IFDIR | 0775;
  root->stat_.uid_ = 0;
  root->stat_.atime_ = now;
  root->stat_.ctime_ = now;
  root->stat_.mtime_ = now;
  root->stat_.block_ = PAGE_SIZE;
  root->dev_ = KALLOC(kDevice_t);
  root->dev_->underlyingDev_ = NULL;
  root->dev_->ino_ = root;
  root->dev_->fs_ = register_driver(TMPFS);
  atomic_inc(&root->dev_->usage_);
  atomic_inc(&root->dev_->fs_->usage_);
  ll_push_back (&kSYS.deviceList_, &root->dev_->allNd_);

  kSYS.rootIno_ = root;

  kSYS.devIno_ = create_inode ("dev", root, S_IFDIR | 0775, 0);
  kSYS.mntIno_ = create_inode ("mnt", root, S_IFDIR | 0775, 0);

  register_driver(GPT);
  register_driver(ISO9660);
  // register_driver(KEYBD);
  // register_driver(VBA);
  // register_driver(FATFS);
  register_driver(ATA);
  // register_driver(BMP); // Usermode
  register_driver(HDD); // Usermode

  mount_alls ();
}


/* ----------------------------------------------------------------------- */
kDriver_t *register_driver(void (*init)(kDriver_t *))
{
  kDriver_t *driver;
  if (init == NULL)
    return NULL;
  driver = KALLOC(kDriver_t);
  init(driver);
  ll_push_back(&kSYS.driverPool_, &driver->allNd_);
  driver->mount(NULL, NULL);
  return driver;
}


/* ----------------------------------------------------------------------- */
kDevice_t *create_device(const char* name, kInode_t* underlying, SMK_stat_t *stat, void* info)
{
  kDevice_t* dev;
  kInode_t * ino;
  kInode_t * dir = kSYS.devIno_;
  kDriver_t *driver = search_driver(stat->major_);

  if (S_ISDIR(stat->mode_))
    dir = kSYS.mntIno_;

  if (!kSYS.sysIno_ && S_ISDIR(stat->mode_) && !strcmpi("sdC", underlying->name_)) {
    ino = register_inode("usr", kSYS.rootIno_, stat, false);
    kSYS.sysIno_ = ino;
  } else
    ino = register_inode(name, dir, stat, false);
  dev = KALLOC(kDevice_t);
  ino->dev_ = dev;
  dev->underlyingDev_ = underlying;
  dev->ino_ = ino;
  dev->fs_ = driver;
  dev->data_ = info;
  ll_push_back (&kSYS.deviceList_, &dev->allNd_);
  kunlock (&ino->lock_);

  // if (!kSYS.sysIno_ && S_ISDIR(stat->mode_) && !strcmpi("sdC", underlying->name_)) {
  // @todo create_link()
  // }

  return dev;
}


/* ----------------------------------------------------------------------- */
int fs_block_read(kInode_t *fp, void* buffer, size_t length, size_t offset)
{
  int err;
  // TODO should be asynchrone
  klock (&fp->lock_);
  err = open_fs (fp);
  if (err)
    return err;
  err = fp->dev_->fs_->read(fp, buffer, length, offset);
  close_fs (fp);
  return err;
}

/* ----------------------------------------------------------------------- */
int RAID0_read(kInode_t *fp, void* buffer, size_t length, size_t offset)
{
  int i, err;
  size_t j;
  kInode_t** inodes = NULL;
  for (i = 0, j = 0; j < length; ++i, j += 512)
    err = fs_block_read(inodes[i], &((char*)buffer)[j], 512, ALIGN_DW((offset + j) / 2, 512));
  return err;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
