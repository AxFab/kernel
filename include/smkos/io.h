#pragma once
#include <smkos/kernel.h>
#include <smkos/sync.h>

#define S_IRUSR     (0400)
#define S_IWUSR     (0200)
#define S_IXUSR     (0100)
#define S_IRWXU     (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRGRP     (S_IRUSR >> 3)
#define S_IWGRP     (S_IWGRP >> 3)
#define S_IXGRP     (S_IXGRP >> 3)
#define S_IRWXG     (S_IRWXU >> 3)
#define S_IROTH     (S_IRGRP >> 3)
#define S_IWOTH     (S_IWGRP >> 3)
#define S_IXOTH     (S_IXOTH >> 3)
#define S_IRWXO     (S_IRWXG >> 3)

#define S_ISUID     (01000)
#define S_ISGID     (02000)
#define S_ISVTX     (04000)

#define S_IALLUGO   (0777)


#define S_IFREG     0x10000
#define S_IFDIR     0x20000
#define S_IFLNK     0x30000
#define S_IFBLK     0x40000
#define S_IFCHR     0x50000
#define S_IFIFO     0x60000
#define S_IFSOCK    0x70000
#define S_IFVOL     0x80000
#define S_IFMT      0xf0000

#define S_ISBLK(m)    (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m)    (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)    (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m)   (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m)    (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)    (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)   (((m) & S_IFMT) == S_IFSOCK)
#define S_ISVOL(m)    (((m) & S_IFMT) == S_IFVOL)


#define S_TYPEISMQ(buf)  (S_ISFIFO(buf->st_mode) && 1)  ///< Test for a message queue
#define S_TYPEISSEM(buf) (S_ISFIFO(buf->st_mode) && 1)  ///< Test for a semaphore
#define S_TYPEISSHM(buf) (S_ISFIFO(buf->st_mode) && 1)  ///< Test for a shared memory object

typedef struct SMK_stat SMK_stat_t;


/** Allow to use `...' as back to containing volume */
#define AT_THREE_DOT (1 << 0)


#define ISO_No 12
#define GPT_No 35
#define ATA_No 41
#define HDD_No 42
#define SVGA_No 23
#define KDB_No 38


/** Structure holding the metadata of an inode
  * @note: This structure will probably be used on user space.
  */
struct SMK_stat {
  long      ino_;     /**< Inode number */
  int       mode_;    /**< File mode */
  int       major_;
  int	      minor_;
  long      uid_;     /**< User ID */
  // long      gid_;  /**< Group ID */
  size_t	  length_;  /**< Length of the file */
  size_t    lba_;     /**< Linear base address */
  time_t    atime_;   /**< Hour of last access */
  time_t    mtime_;   /**< Hour of last update */
  time_t    ctime_;   /**< Hour of last state change */
  size_t    block_;   /**< Number of block on the device */
};



/* DEVICE
  *
  * The kernel keep on cache the entry available on various device mounted on
  * the system.
  */

/** Hold information about file system module.
  * Object are created by the function `register_driver` and
  * destroy on `unregister_driver`
  */
struct kDriver {
  const char *name_;

  int major_;
  struct llnode allNd_;
  atomic_t usage_;
  enum KDR_TYPE type_;

  int (*mount)(kInode_t* dev, const char *name);

  // union {
    //struct { // FILE_SYSTEM / BLOCK
      page_t (*map)(kInode_t *fp, size_t offset, page_t *page);
      int (*sync)(kInode_t *fp, size_t offset, page_t page);

      int (*lookup)(const char *name, kInode_t *dir, SMK_stat_t *file);
	    int (*readdir)();
	    int (*readlink)();
      int (*create)(const char *name, kInode_t *dir, int mode, size_t lg, SMK_stat_t *stat);
    //};
    //struct { // CHAR / PIPE / SOCKET
      int (*read)(kInode_t *fp, void *buffer, size_t length, size_t offset);
      int (*write)(kInode_t *fp, const void *buffer, size_t length, size_t offset);
      int (*flush)(kInode_t *fp);
    //};
  // };
};


struct kDevice {
  kInode_t* ino_;
  kDriver_t *fs_;
  kInode_t* underlyingDev_;
  struct llnode allNd_;
  struct mutex mutex_;
  void* data_;
  atomic_t usage_;
};


struct kInode {
  const char *name_;
  SMK_stat_t stat_;

  int readers_; /**< Counter for the number of place this inode is used. */
  kInode_t *parent_; /**< Parent inode */
  kInode_t *prev_;  /**< Previous inode */
  kInode_t *next_; /**< Next inode */
  kInode_t *child_; /**< First child inode */
  struct spinlock lock_; /**< Lock to control concurrent access. */
  struct llnode lruNd_; /**< Attach an unused inode on the LRU list */
  kDevice_t *dev_;
  struct bbtree pageTree_;

  union {
    kAssembly_t *assembly_;
    kPipe_t *pipe_;
  };
};




void initialize_vfs();
kDriver_t *register_driver(void (*mount)(kDriver_t *));
int open_fs(kInode_t* ino);
int close_fs(kInode_t* ino);
kDevice_t *create_device(const char* name, kInode_t* underlying, SMK_stat_t *stat, void* info);

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    src/core/inode.c
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/** Search an inode on the filetree. */
kInode_t *search_inode (const char *path, kInode_t *dir, int flags);
/** Try to add a new inode on the VFS tree. */
kInode_t *register_inode (const char *name, kInode_t *dir, SMK_stat_t *stat, bool unlock);
/** Create a new inode. */
kInode_t *create_inode(const char* name, kInode_t* dir, int mode, size_t lg);
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes);
/** Function to called to grab an inodes */
int inode_open (kInode_t *ino);
/** Function to release an inodes */
int inode_close (kInode_t *ino);
/** Give the inode a symbolic link is refering to. */
kInode_t *follow_symlink(kInode_t *ino, int *links);


