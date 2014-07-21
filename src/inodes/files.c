#include <inodes.h>
#include <kcpu.h>

// ===========================================================================

int kFs_Open(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_inc_i32(&ino->readers_);
  return __noerror();
}

// ===========================================================================

int kFs_Close(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_dec_i32(&ino->readers_);
  return __noerror();
}

// ===========================================================================

// ---------------------------------------------------------------------------
kInode_t* kFs_MkNode(const char* name, kInode_t* dir, kStat_t* stat)
{
  int err;
  kInode_t* ino;

  if (!dir->fs_->create) {
    __seterrno(EROFS);
    return NULL;
  }

  MOD_ENTER;
  err = dir->fs_->create(name, dir, stat);
  MOD_LEAVE;

  if (err) {
    __seterrno(err);
    return NULL;
  }

  ino = kFs_Register (name, dir, stat);
  if (ino)
    kunlock (&ino->lock_);
  return ino;
}

// ===========================================================================

int kFs_Delete(kInode_t* ino)
{
  return __seterrno(ENOSYS);
}


