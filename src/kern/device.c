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
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/drivers.h>


/* ----------------------------------------------------------------------- */
// static int unregister_driver ()
// {
//   return __seterrno(ENOSYS);
// }


/* ----------------------------------------------------------------------- */
/** @brief Dummy search of a dirver by its major number
  * @retval NULL The driver can't be found
  * @retval kDriver the driver that correspond to this item.
  */
kDriver_t *search_driver(int major)
{
  kDriver_t *driver;
  ll_for_each(&kSYS.driverPool_, driver, kDriver_t, allNd_) {
    if ( driver->major_ == major)
      return driver;
  }

  return NULL;
}


/* ----------------------------------------------------------------------- */
/** @brief Print an inode and all descendants to kernel loging system
  * @param ino The inode that represent the root of the tree to display
  * @param depth Depth of the tree representation. This functions should
  * always be called with a depth of zero.
  */
static void display_inode(kInode_t *ino, int depth)
{
  int i, j;
  kInode_t *dir;

  const char ftype[] = {
    'a', '-', 'd', '3', 'b', 'c', 'p', '7', '8', '9',
  };
  const char *frights[] = {
    "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx", "???",
  };

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

    for (i = 0; i < depth - 1; ++i) {
      j = i;
      dir = ino;

      while (j++ < depth - 1)
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
      display_inode(ino->child_, depth + 1);

    ino = ino->next_;
  }
}


/* ----------------------------------------------------------------------- */
static void dispose_drivers()
{
  kDriver_t *pilote;
  kDriver_t *iter = ll_first(&kSYS.driverPool_, kDriver_t, allNd_);
  while (iter) {
    pilote = iter;
    iter = ll_next(iter, kDriver_t, allNd_);
    if (pilote->major_ != 0)
      unregister_driver(pilote);
  }
}

/* ----------------------------------------------------------------------- */
/** @brief Grab a lock on the file system driver of an inode.
  * @param ino The inode used ot get the lock.
  * @retval ZERO No error occurs and the mutex have been acquired.
  * @note This method must be used only if the inode is locked and the
  * current cpu doesn't have a single other spinlock acquired.
  * The function doesn't have any assertion to avoid deadlock on driver
  * mutex yet.
  */
int open_fs(kInode_t *ino)
{
  assert (kislocked(&ino->lock_));
  assert (kCPU.lockCounter_ == 1);

  mtx_lock(&ino->dev_->mutex_);
  kunlock(&ino->lock_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** @brief Release the mutex after a call to a driver routines
  * @param ino The inode used ot get the lock.
  * @retval ZERO No error occurs and the mutex have been corectrly released.
  */
int close_fs(kInode_t *ino)
{
  // assert (mtx_grabed(&ino->dev_->mutex_));
  assert (kCPU.lockCounter_ == 0);

  mtx_unlock(&ino->dev_->mutex_);
  return __seterrno(0);
}


/* ----------------------------------------------------------------------- */
/** @brief Print all register inodes to kernel loging system */
void display_inodes()
{
  // kprintf(" ACCESS   | x | OWN | DEV  | BLOK | NAME \n");
  display_inode(kSYS.rootIno_, 0);
}


/* ----------------------------------------------------------------------- */
int mount_device(const char *name, kDevice_t *dev, kDriver_t *fs)
{
  int err;
  kDriver_t *driver;

  klock(&dev->ino_->lock_);

  if (open_fs(dev->ino_))
    return __geterrno();

  if (fs == NULL) {
    ll_for_each(&kSYS.driverPool_, driver, kDriver_t, allNd_) {
      if (driver->mount (dev->ino_, name) == 0) {
        close_fs(dev->ino_);
        return 0;
      }
    }
  } else {
    err = fs->mount (dev->ino_, name) == 0;
    close_fs(dev->ino_);
    return __seterrno(err);
  }

  close_fs(dev->ino_);
  return __seterrno(ENOSYS);
}

/* ----------------------------------------------------------------------- */
int unmount_device(kDevice_t *dev)
{
  int err;
  assert (kCPU.lockCounter_ == 1);
  assert (kislocked(&dev->ino_->lock_));

  if (open_fs(dev->ino_))
    return __geterrno();

  if (dev->fs_->unmount) {
    err = dev->fs_->unmount(dev->ino_, dev->data_);

    if (err) {
      close_fs(dev->ino_);
      return __seterrno(err);
    }
  }

  close_fs(dev->ino_);
  klock (&dev->ino_->parent_->lock_);
  klock (&dev->ino_->lock_);

  unregister_inode(dev->ino_);
  atomic_dec(&dev->fs_->usage_);

  if (dev->underlyingDev_) {
    atomic_dec(&dev->underlyingDev_->dev_->usage_);
    inode_close(dev->underlyingDev_);
  }

  ll_remove(&kSYS.deviceList_, &dev->allNd_);
  kfree(dev);
  assert (kCPU.lockCounter_ == 0);
  return __seterrno(0);
}

/* ----------------------------------------------------------------------- */
/** @brief Find all unused devices and ty to mount them against any available
  * driver.
  */
void mount_alls ()
{
  kDevice_t *dev;
  ll_for_each (&kSYS.deviceList_, dev, kDevice_t, allNd_) {
    if (dev->usage_ == 0) {

      mount_device(NULL, dev, NULL);
    }
  }
}


/* ----------------------------------------------------------------------- */
/** @brief Find all devices and unmount all unused one.
  */
void unmount_alls ()
{
  int deleted;
  kDevice_t *dev;
  kDevice_t *iter;

  for (;;) {
    deleted = 0;
    iter = ll_first(&kSYS.deviceList_, kDevice_t, allNd_);

    while (iter) {
      dev = iter;
      iter = ll_next(dev, kDevice_t, allNd_);
      klock(&dev->ino_->lock_);

      if (dev->usage_ == 0 && dev->ino_->readers_ == 0) {
        unmount_device(dev);
        ++deleted;
      } else
        kunlock(&dev->ino_->lock_);
    }

    if (deleted == 0)
      return;
  }
}


/* ----------------------------------------------------------------------- */
void initialize_vfs()
{
  kInode_t *root;
  time_t now = time(NULL);
  // char buf[2048];

  // kprintf ("Initializing virtual file system...\n");

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
  kSYS.procIno_ = create_inode ("proc", root, S_IFDIR | 0555, 0);
  create_inode ("tmp", root, S_IFDIR | 0700, 0);

  init_drivers();
  mount_alls ();
  // display_inodes();
}


/* ----------------------------------------------------------------------- */
void sweep_vfs()
{
  scavenge_inodes(8000);
  unmount_alls();
  dispose_drivers();
  scavenge_inodes(8000);

  if (kSYS.rootIno_->child_ == NULL) {
    atomic_dec(&kSYS.rootIno_->dev_->fs_->usage_);
    atomic_dec(&kSYS.rootIno_->dev_->usage_);
    kfree(kSYS.rootIno_->dev_);
    kfree(kSYS.rootIno_);
    unregister_driver(search_driver(0)); // TMPFS
  } else {
    kprintf("/!\\ Inodes are leaking...\n");
    display_inodes();
  }
}


/* ----------------------------------------------------------------------- */
kDriver_t *register_driver(void (*init)(kDriver_t *))
{
  // @todo check unused
  kDriver_t *driver;
  assert(init != NULL);
  driver = KALLOC(kDriver_t);
  init(driver);
  ll_push_back(&kSYS.driverPool_, &driver->allNd_);
  driver->mount(NULL, NULL);
  return driver;
}

/* ----------------------------------------------------------------------- */
int unregister_driver(kDriver_t *driver)
{
  // @todo first check that no inodes use this driver
  assert(driver != NULL);

  if (driver->usage_ != 0)
    return __seterrno(EBUSY);

  if (driver->dispose)
    driver->dispose();

  ll_remove(&kSYS.driverPool_, &driver->allNd_);

  if (driver->name_)
    kfree((void *)driver->name_);

  kfree(driver);
  return __seterrno(0);
}

/* ----------------------------------------------------------------------- */
kDevice_t *create_device(const char *name, kInode_t *underlying, SMK_stat_t *stat, void *info)
{
  kDevice_t *dev;
  kInode_t *ino;
  kInode_t *dir = kSYS.devIno_;
  kDriver_t *driver = search_driver(stat->major_);

  if (S_ISDIR(stat->mode_))
    dir = kSYS.mntIno_;

  assert (stat->major_ != dir->stat_.major_ || stat->minor_ != dir->stat_.minor_);

  if (!kSYS.sysIno_ && S_ISDIR(stat->mode_) && !strcmpi("sdC", underlying->name_)) {
    ino = register_inode("usr", kSYS.rootIno_, stat, false);
    kSYS.sysIno_ = ino;
  } else
    ino = register_inode(name, dir, stat, false);

  dev = KALLOC(kDevice_t);
  ino->dev_ = dev;

  if (underlying) {
    inode_open(underlying);
    atomic_inc(&underlying->dev_->usage_);
  }

  dev->underlyingDev_ = underlying;
  dev->ino_ = ino;
  dev->fs_ = driver;
  atomic_inc(&driver->usage_);
  dev->data_ = info;
  ll_push_back (&kSYS.deviceList_, &dev->allNd_);
  kunlock (&ino->lock_);

  // if (!kSYS.sysIno_ && S_ISDIR(stat->mode_) && !strcmpi("sdC", underlying->name_)) {
  /// @todo create_link()
  // }

  return dev;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
