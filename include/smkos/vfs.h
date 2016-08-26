
#ifndef _SMKOS_VFS_H
#define _SMKOS_VFS_H 1

#include <smkos/klib.h>
#include <skc/llist.h>
#include <skc/locks.h>
// #include <smkos/types.h>
#include <tchar.h>

typedef struct vfs vfs_t;
typedef struct inode inode_t;
typedef struct dirent dirent_t;
typedef struct device device_t;
typedef struct driver driver_t;

struct vfs {
  dirent_t *root_;
  dirent_t *dev_;
  dirent_t *mnt_;
  dirent_t *sys_;
  dirent_t *usr_;
  atomic_t inoCnt_;
  atomic_t entCnt_;
  llhead_t lru_;
  splock_t lock_;
};

struct inode {
  int no_;
  int mode_;
  int major_;
  int minor_;
  int uid_;
  int gid_;
  device_t *dev_;
  size_t length_;
  size_t lba_;
  size_t block_;
  time_t atime_;
  time_t ctime_;
  time_t mtime_; 
};

struct dirent {
  TCHAR *name_;
  atomic_t readers_;
  inode_t *ino_;
  dirent_t *parent_;
  dirent_t *prev_;
  dirent_t *next_;
  dirent_t *child_;
  rwlock_t lock_;
  llnode_t lruNd_;
  device_t *dev_;
  
  union {
    dirent_t *slink_;
    // fifo_t *pipe_;
    // assembly_t *asm_;
  };
};

struct device
{
  inode_t *ino_;
  driver_t *drv_;
  device_t *dev_;
  llnode_t allNd_;
  mutex_t mtx_;
  atomic_t inoCnt_;
  void *data_;
};

struct driver
{
  TCHAR name_[8];
  int major_;
  int flags_;
  llnode_t allNd_;
  mutex_t mtx_;
  atomic_t inoCnt_;

  int (*compare)(const TCHAR *str1, const TCHAR *str2);

  int (*mount)(inode_t *dev, const char *name);
  int (*unmount)(inode_t* dev, void* info);
  int (*dispose)();

  int (*map)(inode_t *fp, size_t offset, page_t *page);
  int (*sync)(inode_t *fp, size_t offset, page_t page);

  int (*lookup)(const TCHAR *name, inode_t *dir, inode_t *ino);
  int (*readlink)(inode_t *fp, TCHAR* path, int lg);
  
  int (*readdir)();

  int (*mknod)(const TCHAR *name, inode_t *dir, inode_t *ino);
  int (*create)(const char *name, inode_t *dir, int mode, size_t lg, inode_t *stat);
  int (*symlink)(const char *name, inode_t *dir, int mode, const char *path, inode_t *stat);

  int (*read)(inode_t *fp, void *buffer, size_t length, size_t offset);
  int (*write)(inode_t *fp, const void *buffer, size_t length, size_t offset);
  int (*flush)(inode_t *fp);

  int (*release)(inode_t *fp);
};


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define AT_THREE_DOT (1 << 0)


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern struct vfs __VFS;


/* Inistialize virtual file system */
void vfs_init();
/* Desallocate virtual file system */
void vfs_sweep();
/* Call the inode scavenger which will try to free cached data */
void vfs_scavenge(int count);
/* */
void vfs_close(dirent_t *ent);
/* */
void vfs_display();


/* Search for the child of a node */
dirent_t *search_child(const TCHAR *name, dirent_t *dir);
/* Search an inode on the file-tree */
dirent_t *vfs_lookup(const TCHAR *path, dirent_t *ref, int flags);
/* Dereference an inode return by `lookup' function */
void vfs_release(dirent_t *ent);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Refresh inode info (used for distributed data) */
int drvfs_refresh(inode_t *ino); 
/* Free inode-related driver meta-data */
int drvfs_release(inode_t *ino);
/* Read data from block, size and offset should be align on block size */
size_t drvfs_readblk(inode_t * ino, void *buf, size_t lg, off_t off);
/* Write data on block, size and offset should be align on block size */
size_t drvfs_writeblk(inode_t * ino, const void *buf, size_t lg, off_t off);
/* Either allocate or truncate the file */
int drvfs_resize(inode_t *ino, off_t size);  
/* Stream read -- (can blocked) */
size_t drvfs_read(inode_t * ino, void *buf, size_t lg, int flags);
/* Stream write */
size_t drvfs_write(inode_t * ino, const void *buf, size_t lg, int flags);
/* Search for the presence of an inode on the device */
dirent_t* drvfs_lookup(const TCHAR *name, dirent_t* dir);
/* Create a new inode */
dirent_t* drvfs_mknod(const TCHAR *name, dirent_t* dir, int mode);
/* Remove an existing inode */ 
int drvfs_rmnod(const TCHAR *name, inode_t* dir, inode_t *ino);
/*  */
int drvfs_link(const TCHAR *name, inode_t* dir, inode_t *ino);
/* */
int drvfs_unlock(const TCHAR *name, inode_t* dir, inode_t *ino);
/* Remove all of children contains by the inode */
int drvfs_dropall(inode_t *ino);
/* Create an iterator for files */
void *drvfs_opendir(inode_t *dir);
/* Iterate over directory  */
int drvfs_readdir(void *it, inode_t *ino, const TCHAR *buf, int lg);
/*  */
int drvfs_flush(inode_t *ino);
/* */
int drvfs_prompt(inode_t *ino, const TCHAR *value); 

int drvfs_map(inode_t *ino, page_t *pg, off_t off, int psz);

int drvfs_flip(inode_t *ino);
// /* Create a record type of inode */
// int drvfs_mkrcd(const TCHAR *name, inode_t* dir, inode_t *ino, const void *buf, int lg);
// /* */
// int drvfs_getrcd(inode_t *ino, const TCHAR *buf, size_t lg);

int drvfs_readlink(inode_t *ino, TCHAR *buf, int lg);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

void TMPFS_init(driver_t *drv);

int TMPFS_mknod(const TCHAR *name, inode_t* dir, inode_t *ino);

#endif  /* _SMKOS_VFS_H */
