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
#define S_IFTTY     0x80000
#define S_IFMT      0xf0000

#define S_ISBLK(m)    (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m)    (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)    (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m)   (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m)    (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)    (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)   (((m) & S_IFMT) == S_IFSOCK)
#define S_ISTTY(m)    (((m) & S_IFMT) == S_IFTTY)


#define S_TYPEISMQ(buf)  (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a message queue */
#define S_TYPEISSEM(buf) (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a semaphore */
#define S_TYPEISSHM(buf) (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a shared memory object */

typedef struct SMK_stat SMK_stat_t;


/** Allow to use `...' as back to containing volume */
#define AT_THREE_DOT (1 << 0)



/** Structure holding the metadata of an inode
  * @note: This structure will probably be used on user space.
  */
struct SMK_stat {
  long      ino_;     /**< Inode number */
  int       mode_;    /**< File mode */
  int       major_;
  int       minor_;
  long      uid_;     /**< User ID */
  /* long      gid_;  / **< Group ID */
  size_t    length_;  /**< Length of the file */
  size_t    lba_;     /**< Linear base address */
  time_t    atime_;   /**< Hour of last access */
  time_t    mtime_;   /**< Hour of last update */
  time_t    ctime_;   /**< Hour of last state change */
  size_t    block_;   /**< Number of block on the device */
};




