#include <inodes.h>
#include <kinfo.h>


kInode_t* kFs_RootInode()
{
  return kSYS.RootFs;
}

#define FS_DEV_NODE "dev"
#define FS_MNT_NODE "mnt"

extern kFileOp_t tmpFsOperation;

// ---------------------------------------------------------------------------
/** Initialize the FS kernel module 
 *  Create the root node and start device detection
 *  @note Requests a list of device drivers.
 */
int kFs_Initialize() 
{
  time_t now = time(NULL);
  kInode_t* root = KALLOC(kInode_t);
  kSYS.RootFs = root;
  root->stat_.ino_ = kSys_NewIno();
  root->stat_.mode_ = S_IFDIR | 0755;
  root->stat_.atime_ = now;
  root->stat_.mtime_ = now;
  root->stat_.ctime_ = now;
  root->stat_.dblock_ = 1;
  root->stat_.cblock_ = PAGE_SIZE;
  root->fs_ = &tmpFsOperation;

  kSYS.devNd_ = kFs_MkNode (FS_DEV_NODE, root, &root->stat_);
  kSYS.mntNd_ = kFs_MkNode (FS_MNT_NODE, root, &root->stat_);

  ATA_Initialize (kSYS.devNd_);

  return __noerror();
}


// ---------------------------------------------------------------------------
int kFs_CreateDevice (const char* name, kInode_t* dir, kFileOp_t* fileops, void* devinfo, kStat_t* stat)
{
  stat->atime_ = time(NULL);
  kInode_t* dev = kFs_Register (name, dir, stat);
  if (!dev) 
    return __geterrno();

  dev->fs_ = fileops;
  dev->devinfo_ = devinfo;
  kunlock (&dev->lock_);
  return __noerror();
}


ssize_t kFs_Read(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (ino != NULL && buffer != NULL);

  if (ino->fs_->read == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  kprintf ("FS] Read '%s' on LBA %d using %x\n", ino->name_, offset, ino->fs_->read);
  int err = ino->fs_->read (ino, buffer, length, offset);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return length;
}


