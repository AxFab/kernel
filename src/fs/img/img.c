#include <kernel/inodes.h>
#include <stdio.h>


int img_Read(dev_t fd, void* buffer, off_t offset, size_t length)
{
  lseek (fd, offset, SEEK_SET);
  read(fd, buffer, length);
  return 0;
}

int img_Write(dev_t fd, void* buffer, off_t offset, size_t length)
{
  lseek (fd, offset, SEEK_SET);
  write(fd, buffer, length);
  return 0;
}



int img_InitBlock (kDevice_t* dev)
{
  if (!dev->fd_)
    return ENOTBLK;

  dev->block_ = 2048;
  dev->size_ = lseek(dev->fd_, 0, SEEK_END);
  dev->read = img_Read;
  dev->write = NULL;
  strncpy (dev->name_, "Image driver", 16);
  return 0;
}

int isoMount (kFsys_t* fs, kStat_t* root);

int img_Initialize (kInode_t* dev, kInode_t* mnt)
{
  int fd = open ("../kernel/Os.iso", O_RDWR);

  if (!fd) return EBADF;

  kInode_t* blk = kFs_CreateBlock ("iso", dev, fd, img_InitBlock);
  // FIXME Handle plug N play better
  kFs_Mount ("cdrom", mnt, blk, isoMount);
  return __noerror();
}






