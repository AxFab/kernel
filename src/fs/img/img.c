#include <kernel/vfs.h>
#include <stdio.h>
#include <fcntl.h>


int IMG_read (kInode_t *ino, void *buf, size_t count, size_t lba);
int IMG_write (kInode_t *ino, const void *buf, size_t count, size_t lba);

struct img_device {
  kDevice_t dev_;
  int fd_;
};
// kDevice_t imgOps = {
//   NULL, NULL, NULL,
//   NULL, IMG_read, NULL, NULL,
//   NULL, IMG_write, NULL, NULL, NULL,
//   NULL,
// };


int IMG_read (kInode_t *ino, void *buf, size_t count, size_t lba)
{
  struct img_device *dr = (struct img_device *)ino->dev_;
  lseek (dr->fd_, lba * ino->stat_.block_, SEEK_SET);
  read(dr->fd_, buf, count * ino->stat_.block_);
  return 0;
}


int IMG_write (kInode_t *ino, const void *buf, size_t count, size_t lba)
{
  struct img_device *dr = (struct img_device *)ino->dev_;
  lseek (dr->fd_, lba * ino->stat_.block_, SEEK_SET);
  write(dr->fd_, buf, count * ino->stat_.block_);
  return 0;
}

int IMG_init (kInode_t *dev)
{
  int fd;
  kStat_t stat = { 0 };
  stat.mode_ =  S_IFBLK | 0755;

  fd = open ("Os.iso", O_RDWR);

  if (fd) {
    stat.block_ = 2048;
    struct img_device *devStruct = (struct img_device *)kalloc(sizeof (struct img_device), 0);
    devStruct->fd_ = fd;
    devStruct->dev_.read = IMG_read;
    devStruct->dev_.write = IMG_write;
    create_device("sdA", dev, (kDevice_t *)devStruct, &stat);
    // kfs_new_device ("sdA", dev, &imgOps, (void*)fd, &stat);
  }


  fd = open ("Hdd.img", O_RDWR | O_CREAT);

  if (fd) {
    stat.block_ = 512;
    struct img_device *devStruct = (struct img_device *)kalloc(sizeof (struct img_device), 0);
    devStruct->fd_ = fd;
    devStruct->dev_.read = IMG_read;
    devStruct->dev_.write = IMG_write;
    create_device("sdC", dev, (kDevice_t *)devStruct, &stat);
    // kfs_new_device ("sdC", dev, hdd_device, (void*)fd, &stat);
  }

  return 0;
}



