/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Initialization of the module for inodes.
 */
#include <kernel/inodes.h>
#include <kernel/info.h>


extern kFileOp_t tmpFsOperation;

// ===========================================================================
/** Initialize the FS kernel module
 *  Create the root node and start device detection
 *  @note Requests a list of device drivers.
 */
int kfs_init()
{
  time_t now = time(NULL);
  kInode_t* root = KALLOC(kInode_t);
  kSYS.rootNd_ = root;
  root->stat_.ino_ = kSys_NewIno();
  root->stat_.mode_ = S_IFDIR | 0755;
  root->stat_.atime_ = now;
  root->stat_.mtime_ = now;
  root->stat_.ctime_ = now;
  root->stat_.dblock_ = 1;
  root->stat_.cblock_ = PAGE_SIZE;
  root->fs_ = &tmpFsOperation;

  kSYS.devNd_ = kfs_mknod (FS_DEV_NODE, root, &root->stat_);
  kSYS.mntNd_ = kfs_mknod (FS_MNT_NODE, root, &root->stat_);

  return __noerror();
}


// ---------------------------------------------------------------------------
/** */
int kfs_new_device (const char* name, kInode_t* dir, kFileOp_t* fileops, void* devinfo, kStat_t* stat)
{
  stat->atime_ = time(NULL);
  kInode_t* dev = kfs_register (name, dir, stat);
  if (!dev)
    return __geterrno();

  dev->fs_ = fileops;
  dev->devinfo_ = devinfo;
  kunlock (&dev->lock_);
  return __noerror();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
