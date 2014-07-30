#ifndef INODES_H__
#define INODES_H__

#include <kernel/core.h>

#include <sys/stat.h>
#include <fcntl.h>

#define S_IALLUGO (0777)

// ============================================================================


struct kStat {
  ino_t           ino_;       ///< Inode number
  mode_t          mode_;      ///< File mode
  uid_t           uid_;       ///< User ID
  gid_t           gid_;       ///< Group ID
  uint64_t        length_;    ///< Length of the file
  uint64_t        lba_;       ///< Linear base address
  time_t          atime_;     ///< Hour of last access
  time_t          mtime_;     ///< Hour of last update
  time_t          ctime_;     ///< Hour of last state change
  ino_t           dev_;       ///< Device handle
  size_t          dblock_;
  size_t          cblock_;
};


struct kInode {
  const char*     name_;      ///< Name of the inode
  kStat_t         stat_;      ///<
  int             readers_;   ///< Usage counter
  kInode_t*       parent_;    ///< Parent inode
  kInode_t*       prev_;      ///< Previous inode
  kInode_t*       next_;      ///< Next inode
  kInode_t*       child_;     ///< First child inode
  spinlock_t      lock_;      ///< Lock
  kFileOp_t*      fs_;        ///< File system driver
  union {
    kAssembly_t*    assembly_;
    kFifoPen_t*     fifo_;
  };
  void*           devinfo_;
  int             pageCount_;  ///< max number of physical pages in cache
  kPage_t*        pagesCache_; ///< physical pages caching
};

struct kPage
{
  uint32_t  phys_;
  off_t     offset_;
  int       flags_;
};


struct kFileOp {

  // read / write / poll / ioctl / map / allocate
  // getxattr / setxattr / listxattr

  // lookup / create(mkdir) / remove(rmdir) / rename / setmeta
  // follow_link / symlink
  // setacl / getacl
  // link / unlink... (for compatibility)

  // mount / umount / statfs / check / ioctl
  // format
  // defrag !?


  int (*mount)(kInode_t* dev, kInode_t* mnt);
  int (*umount)();
  int (*format)(dev_t dev_, const char* options);
  // freeze / unfreeze / statfs / umount / options

  int (*lookup)(const char* name, kInode_t* dir, kStat_t* file);
  int (*read)(kInode_t* fp, void* buffer, size_t length, size_t offset);
  int (*readdir)();
  int (*readlink)();

  int (*create)(const char* name, kInode_t* dir, kStat_t* file);
  int (*write)(kInode_t* fp, void* buffer, size_t length, size_t offset);
  int (*link)();
  int (*unlink)();
  int (*symlink)();

  uint32_t (*map)(kInode_t* fp, off_t offset);
};



struct kDevice {
  int (*read)(dev_t fd, void* buffer, off_t offset, size_t length);
  int (*write)(dev_t fd, void* buffer, off_t offset, size_t length);
  char name_ [16];
  size_t block_;
  size_t size_;
  dev_t fd_;
  //
};

struct kResxFile {
  //
};

// ============================================================================

int kfs_init();

kInode_t* kfs_lookup(const char* path, kInode_t* dir);
kInode_t* kfs_register(const char* name, kInode_t* dir, kStat_t* stat);
int kfs_unregister(kInode_t* ino);

int kfs_grab(kInode_t* ino);
int kfs_release(kInode_t* ino);
kInode_t* kfs_mknod(const char* name, kInode_t* dir, kStat_t* stat);
int kFs_Delete(kInode_t* ino);

ssize_t kFs_Read(kInode_t* ino, void* buffer, size_t count, off_t offset);
ssize_t kFs_Write(kInode_t* ino, void* buffer, size_t count, off_t offset);

// int kFs_Read(kInode_t* ino, void* buffer, off_t offset, size_t count);
// int kFs_Write(kInode_t* ino, void* buffer, off_t offset, size_t count);
int kFs_Sync(kInode_t* ino);

int kfs_follow_link (kInode_t** ino, int* loopCount);
int kfs_plink (kInode_t* ino, char* ptr, size_t length);
int kfs_puri (kInode_t* ino, char* ptr, size_t length);
void kfs_log_ino (kInode_t* ino, int depth);
void kfs_log_all ();

// kInode_t* kFs_Mount (const char* name, kInode_t* dir, kInode_t* dev, int (*fsEntry)(kFsys_t* fs, kStat_t* file));
// kInode_t* kFs_CreateBlock (const char* name, kInode_t* dir, dev_t fd, int (*driverEntry)(kDevice_t* dev));

int kfs_new_device (const char* name, kInode_t* dir, kFileOp_t* fileops, void* devinfo, kStat_t* stat);

// int kFs_CreateBlock (const char* name, kFileOp_t* fileops, void* devinfo, size_t block);


// int kFs_ReadBlock (kDevice_t* dev, void* buffer, off_t offset, size_t count);
// int kFs_Feed(kInode_t* ino, void* buffer, off_t offset, size_t count);


int kfs_feed(kInode_t* ino, void* buffer, size_t length, off_t offset);
int kfs_sync(kInode_t* ino, void* buffer, size_t length, off_t offset);
int kfs_map (kInode_t*ino, off_t offset, uint32_t* page, int* mustRead);

#endif /* INODES_H__ */

