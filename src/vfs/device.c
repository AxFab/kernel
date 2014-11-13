/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to handle devices operations.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Create and register a new device. */
id_t create_device(const char* nm, kInode_t* dir, kDevice_t* dev, kStat_t* stat)
{
  assert (PARAM_FILENAME (nm));
  assert (PARAM_KOBJECT (dir, kInode_t));
  assert (PARAM_KOBJECT (dev, kDevice_t));
  assert (PARAM_KOBJECT (stat, kStat_t));

  static id_t auto_inc = 0;
  stat->atime_ = stat->ctime_ = stat->mtime_ = time(NULL);
  dev->id_ = ++auto_inc;

  klock(&dir->lock_);
  kInode_t* ino = register_inode (nm, dir, stat);
  if (!dev)
    return 0;

  dev->ino_ = ino;
  ino->dev_ = dev;
  ino->stat_.dev_ = dev->id_;
  kunlock (&ino->lock_);
  return dev->id_;
}


// ---------------------------------------------------------------------------
/** Search for device by it's handle. */
kInode_t* search_device(dev_t dev)
{

}


// ---------------------------------------------------------------------------
/** Try to initalize a driver for a specific device. */
int mount_device (kInode_t* dev, const char* name, kInode_t* mnt, int fs,
                  int flags, const char* data)
{

}


// ---------------------------------------------------------------------------
/** Release a device and close it's driver program. */
int umount_device(kInode_t* dev)
{

}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
