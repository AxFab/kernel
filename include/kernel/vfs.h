/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Header file of the virtual file system module.
 *      Create a file hierarchie and make used of IO ressources.
 */
#ifndef KERNEL_VFS_H__
#define KERNEL_VFS_H__

#include <kernel/core.h>

// ===========================================================================
//      Definitions and Macros
// ===========================================================================
#include <sys/stat.h>  // S_*
#include <fcntl.h>     // O_*, AT_*

#ifndef S_ISTTY
#  define S_IFTTY S_IFSOCK // Error, but we don't use them yet...
#  define S_ISTTY(m)  (((m) & S_IFMT) == S_IFTTY)
#endif

extern kDevice_t tmpFs;

// ===========================================================================
//      Data Structures
// ===========================================================================
/** Structure holding the metadata of an inode 
  * @note: This structure will probably be used on user space.
  */
struct kStat 
{
  id_t            ino_;       ///< Inode number
  mode_t          mode_;      ///< File mode
  id_t            uid_;       ///< User ID
  // id_t            gid_;       ///< Group ID
  uint64_t        length_;    ///< Length of the file
  uint64_t        lba_;       ///< Linear base address
  time_t          atime_;     ///< Hour of last access
  time_t          mtime_;     ///< Hour of last update
  time_t          ctime_;     ///< Hour of last state change
  id_t            dev_;       ///< Device handle
  size_t          block_;     ///< Number of block on the device
};


// ---------------------------------------------------------------------------
/** Structure containing all system information relative to an inode.
  * @todo Think about a clever way to store the name of the inode.
  */
struct kInode 
{
  // K_OBJECT;
  const char*     name_;      ///< Name of the inode
  kStat_t         stat_;      ///<
  int             readers_;   ///< Usage counter
  kInode_t*       parent_;    ///< Parent inode
  kInode_t*       prev_;      ///< Previous inode
  kInode_t*       next_;      ///< Next inode
  kInode_t*       child_;     ///< First child inode
  spinlock_t      lock_;      ///< Lock
  kDevice_t*      dev_;       ///< Device driver
  union {
    kAssembly_t*  assembly_;
    kFifo_t*      fifo_;
    kTerm_t*      term_;
  };
  int             pageCount_;  ///< max number of physical pages in cache
  kBucket_t*      pagesCache_; ///< physical pages caching
  anchor_t        buckets_;
  anchor_t        evList_;      /// Events list
  list_t          lruNd_;    /// LRU list
};


// ---------------------------------------------------------------------------
struct kDevice 
{
  // K_OBJECT;
  id_t id_;
  spinlock_t      lock_;      ///< Lock
  list_t          all_;      ///< List of all devices.
  kInode_t*       ino_;

  // poll / ioctl / map / allocate - getxattr / setxattr / listxattr
  // lookup / create(mkdir) / remove(rmdir) / rename / setmeta
  // follow_link / symlink -  setacl / getacl
  // link / unlink... (for compatibility)
  // mount / umount / statfs / check / ioctl
  // format -  defrag !?
  // int (*format)(dev_t dev_, const char* options);
  // freeze / unfreeze / statfs / umount / options

  int (*lookup)(const char* name, kInode_t* dir, kStat_t* file);
  int (*read)(kInode_t* fp, void* buffer, size_t length, size_t offset);
  int (*readdir)();
  int (*readlink)();

  int (*create)(const char* name, kInode_t* dir, kStat_t* file);
  int (*write)(kInode_t* fp, const void* buffer, size_t length, size_t offset);

  uint32_t (*map)(kInode_t* fp, off_t offset);
};


// ---------------------------------------------------------------------------
/** A bucket is a memory cache for all or part of a file.
  * @note: A bucket must be of size (PAGE_SIZE) or (K x 8 x PAGE_SIZE).
  */
struct kBucket 
{
  // K_OBJECT;
  page_t    phys_;    ///< Physique address of this page bucket.
  size_t    length_;  ///< Length of the page bucket.
  off_t     offset_;  ///< Offset on the file.
  int       flags_;   ///< Flags for this bucket properties.
  list_t    node_;    ///< List of bucket for Last-Recently-Used policy.
};



// ===========================================================================
//      Methods
// ===========================================================================

// VFS/DATA ==================================================================
/** Request the file system to feed the inode page buffer. */
int feed_inode(kInode_t* ino, void* buffer, size_t length, off_t offset);
/** Request the file system to synchronize against the inode page buffer. */
int sync_inode(kInode_t* ino, const void* buffer, size_t length, off_t offset);
/** Find a memory bucket for the content of an inode. */
kBucket_t* inode_bucket(kInode_t* ino, off_t offset);
/** Find a physique page for the content of an inode. */
int inode_page(kInode_t* ino, off_t offset, page_t* page);


// VFS/DEVICE ================================================================
/** Create and register a new device. */
id_t create_device(const char* nm, kInode_t* ino, kDevice_t* dev, 
                   kStat_t* stat);
/** Search for device by it's handle. */
kDevice_t* search_device(id_t dev);
/** Try to initalize a driver for a specific device. */
int mount_device (kInode_t* dev, const char* name, kInode_t* mnt, int fs,
                  int flags, const char* data);
/** Release a device and close it's driver program. */
int umount_device(kInode_t* dev);


// VFS/PATH ==================================================================
/** Read an inode full pathname. */
ssize_t read_pathname(kInode_t* ino, int method, char* buf, int size);
/** Read the content of a directory. */
int read_directory(kInode_t* dir, off_t offset, void* buf, size_t size);


// VFS/REGISTER ==============================================================
/** Try to add a new inode on the VFS tree. */
kInode_t* register_inode (const char* name, kInode_t* dir, kStat_t* stat);
/** Release an inode form the inode cache. */
int unregister_inode (kInode_t* ino);
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes);
/** Call the bucket scavanger which will try to free cached page. */
int scavenge_bucket(int pages);
/** Function to called to grab an inodes */
int inode_open (kInode_t* ino);
/** Function to release an inodes */
int inode_close (kInode_t* ino);

// VFS/SEARCH ================================================================
/** Search an inode on the filetree. */
kInode_t* search_inode (const char* path, kInode_t* dir);
/** Give the inode a symbolic link is refering to. */
kInode_t* follow_symlink(kInode_t* ino, int* links);


// VFS/TMPFS =================================================================
/** Initialize the VFS kernel module */
int initialize_vfs();


// VFS/WMETA =================================================================
/** Request the file system for the creation of a new inode. */
kInode_t* create_inode(const char* name, kInode_t* dir, mode_t mode, size_t lg);
/** Request the file system to remove an inode. */
int remove_inode(kInode_t* ino);
/** Request the file system to update the inode metadata. */
int chmeta_inode(kInode_t* ino, kStat_t* stat);
/** Request the file system to change the path of the inode. */
int rename_inode(kInode_t* ino, const char* name, kInode_t* dir);


// LEGACY - to remove
// ---------------------------------------------------------------------------
int kfs_grab(kInode_t* ino);
int kfs_release(kInode_t* ino);


#endif /* KERNEL_VFS_H__ */
