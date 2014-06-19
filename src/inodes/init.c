#include <inodes.h>
#include <kinfo.h>


kInode_t* kFs_RootInode()
{
  return kSYS.RootFs;
}

int tmpfsMount (kFsys_t* fs);

int kFs_Initialize ()
{
  int err;
  kInode_t* root = KALLOC(kInode_t);
  kSYS.RootFs = root;
  root->stat_.no_ = (int)root;
  root->stat_.mode_ = S_IFDIR | 0755;
  root->stat_.uid_ = ROOT_UID;
  root->stat_.gid_ = ROOT_UID;
  root->fs_ = KALLOC(kFsys_t);
  err = tmpfsMount (root->fs_);

  if (err)
    return __seterrno (err);

  return __noerror();
}


