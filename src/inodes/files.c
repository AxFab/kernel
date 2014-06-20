#include <inodes.h>

// ===========================================================================

int kFs_Open(kInode_t* ino)
{
  if (!ino) 
    return __seterrno (EINVAL);

  klock(&ino->lock_);
  ++ino->readers_;
  kunlock (&ino->lock_);
  return __noerror();
}

// ===========================================================================

int kFs_Close(kInode_t* ino)
{
  if (!ino) 
    return __seterrno (EINVAL);
  
  klock(&ino->lock_);
  --ino->readers_;
  kunlock (&ino->lock_);
  return __noerror();
}

// ===========================================================================

kInode_t* kFs_MkNode(const char* name, kInode_t* dir, kStat_t* stat)
{
  kInode_t* ino;

  if (!dir->fs_->create) {
    __seterrno(EROFS);
    return NULL;
  }

  if (!dir->fs_->create(name, dir, stat)) {
    ino = kFs_Register (name, dir, stat);

    if (ino) kunlock (&ino->lock_);

    return ino;
  }

  return NULL;
}

// ===========================================================================

int kFs_Delete(kInode_t* ino)
{
  return __seterrno(ENOSYS);
}


