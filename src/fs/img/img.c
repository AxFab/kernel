#include <kernel/vfs.h>
#include <stdio.h>
#include <fcntl.h>


int IMG_read (kInode_t* ino, void* buf, size_t count, size_t lba);
int IMG_write (kInode_t* ino, void* buf, size_t count, size_t lba);


kDevice_t imgOps = {
  NULL, NULL, NULL,
  NULL, IMG_read, NULL, NULL,
  NULL, IMG_write, NULL, NULL, NULL,
  NULL,
};


int IMG_read (kInode_t* ino, void* buf, size_t count, size_t lba)
{
  int fd = (int)ino->devinfo_;
  lseek (fd, lba * ino->stat_.dblock_, SEEK_SET);
  read(fd, buf, count * ino->stat_.dblock_);
  return 0;
}


int IMG_write (kInode_t* ino, void* buf, size_t count, size_t lba)
{
  int fd = (int)ino->devinfo_;
  lseek (fd, lba * ino->stat_.dblock_, SEEK_SET);
  write(fd, buf, count * ino->stat_.dblock_);
  return 0;
}

extern const char* sysvolume_name;

int IMG_init (kInode_t* dev)
{
  int fd;
  time_t now = time (NULL);
  kStat_t stat = { 0, S_IFBLK | 0755, 0, 0, 0L, 0L, now, now, now, 0, 0, 0 };

  fd = open ("Os.iso", O_RDWR);
  if (fd) {
    stat.dblock_ = stat.cblock_ = 2048;
    kfs_new_device ("sdA", dev, &imgOps, (void*)fd, &stat);
  }


  fd = open ("Hdd.img", O_RDWR | O_CREAT);
  if (fd) {
    stat.dblock_ = stat.cblock_ = 512;
    kfs_new_device ("sdC", dev, &imgOps, (void*)fd, &stat);
  }

  sysvolume_name = "sdA";



  return 0;
}



