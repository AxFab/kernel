#include <inodes.h>


int kFs_Read(kInode_t* ino, void* buffer, off_t offset, size_t count)
{
  if (!ino->physPages_) {
    ino->pageCount_ = ino->stat_.length_ / PAGE_SIZE;
    ino->physPages_ = (uint32_t*)kalloc(sizeof(uint32_t) * ino->pageCount_);
  }

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
  memset (buffer, 0x99, count);

  if (S_ISBLK(ino->stat_.mode_)) {
    if (!ino->stat_.dev_)
      return __seterrno(EPERM);

    return kFs_ReadBlock (ino->stat_.dev_, buffer, offset, count);

  } else {
    if (!ino->fs_->read)
      return __seterrno(ENOSYS);

    ino->fs_->read (ino, buffer, offset, count);
    // kprintf ("DEBUG WANT TO FEED \n" /*%s, <%x-%x>\n", ino->name_, offset, count */);
  }

  return __noerror();
}

