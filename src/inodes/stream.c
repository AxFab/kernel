#include <inodes.h>

// Read a fixed size file
static int kFs_ReadBlockFile (kInode_t* ino, void* buffer, size_t count, off_t offset) 
{
  if (offset < 0 || ((size_t)offset) > ino->stat_.length_)
    return __seterrno (ERANGE);

  if (count + offset > ino->stat_.length_) 
    count = ino->stat_.length_ - offset;

  if (!ino->physPages_) {
    ino->pageCount_ = ino->stat_.length_ / PAGE_SIZE;
    ino->physPages_ = (uint32_t*)kalloc(sizeof(uint32_t) * ino->pageCount_);
  }

  //int block = 8092;   // MAX (KREAD_MAP_SIZE, block / cluster)
  // kVma_Mmap ();
  kFs_Feed (ino, buffer, offset, count);

  return __noerror();
}


// No seek, unlimited size, may-block
static int kFs_ReadCharStream (kInode_t* ino, void* buffer, size_t count) 
{

  return __seterrno(ENOSYS);
}

int kFs_Read(kInode_t* ino, void* buffer, off_t offset, size_t count)
{
  if (S_ISBLK(ino->stat_.mode_) || S_ISREG(ino->stat_.mode_)) 
    return kFs_ReadBlockFile(ino, buffer, count, offset);

  else if (S_ISCHR(ino->stat_.mode_) || S_ISFIFO(ino->stat_.mode_) || S_ISSOCK(ino->stat_.mode_)) 
    return kFs_ReadCharStream(ino, buffer, count);

  else // DIR and LNK can be read here
    return __seterrno(ENOSYS);

  return __noerror();
}


int kFs_Write(kInode_t* ino, void* buffer, off_t offset, size_t count)
{
  return __noerror();
}

int kFs_Sync(kInode_t* ino)
{
  return __noerror();
}

// ============================================================================


int kFs_ReadBlock (kDevice_t* dev, void* buffer, off_t offset, size_t count)
{
  int err;

  if (!dev->read)
    return __seterrno(ENOSYS);

  MOD_ENTER;
  err = dev->read (dev->fd_, buffer, offset, count);
  MOD_LEAVE;
  return __seterrno (err);
}


int kFs_Feed(kInode_t* ino, void* buffer, off_t offset, size_t count)
{
  int err;
  memset (buffer, 0x99, count);

  if (!ino)
    return __seterrno (EINVAL);

  if (S_ISBLK(ino->stat_.mode_)) {
    if (!ino->stat_.dev_)
      return __seterrno(EPERM);

    return kFs_ReadBlock (ino->stat_.dev_, buffer, offset, count);

  } else {
    if (!ino->fs_->read)
      return __seterrno(ENOSYS);

    MOD_ENTER;
    err = ino->fs_->read (ino, buffer, offset, count);
    MOD_LEAVE;
    if (err)
      return __seterrno(err);
  }

  return __noerror();
}

