#ifndef _SRC_VFS_H 
#define _SRC_VFS_H 1

#include <skc/llist.h>
#include <skc/splock.h>

/* -- */
typedef struct stat stat_t;
struct stat 
{
  int mode_;
  size_t lba_;
  size_t length_;
  size_t size_;
};

char *strtok_r(const char*, const char *, char **);
// void free(void *);

#ifndef MAX
# define MIN(a,b) ((a)<(b)?(a):(b))
# define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/* -- */

typedef struct inode inode_t;
typedef struct device device_t;
typedef struct driver driver_t;

struct inode 
{
  char *name_;
  rwlock_t lock_;
  stat_t stat_;
  device_t *dev_;
  inode_t *parent_;
  inode_t *first_child_;
  inode_t *prev_sibling_;
  inode_t *next_sibling_;
  
  int usage_;

  union {
    inode_t *symlink_;
  } s;
};

struct device 
{
  

  device_t *sdev_;
  driver_t *drv_;
};

struct driver
{
  int major_;
  int case_sensitive_;
  
  int (*lookup)(const char*, inode_t*, stat_t*);
  int (*readlink)(inode_t*, char*, int);
};



struct vfs 
{
  inode_t *root_ino_;
  inode_t *pwd_ino_;
};

extern struct vfs ___vfs;
#define __VFS (___vfs)
#define __PWD __VFS.pwd_ino_



#define S_IFMSK (15 << 16)
#define S_IFLNK (1 << 16)
#define S_IFREG (2 << 16)
#define S_IFDIR (3 << 16)

#define S_ISLNK(m)  ((m & S_IFMSK) == S_IFLNK)
#define S_ISREG(m)  ((m & S_IFMSK) == S_IFREG)
#define S_ISDIR(m)  ((m & S_IFMSK) == S_IFDIR)

#define AT_THREE_DOT (1 << 0)
#define AT_GET_PARENT (1 << 1)

#define AT_SYMLINK   (AT_THREE_DOT)

#define PATH_MAX 8192

/* lookup.c */
inode_t *inode_lookup(const char *path, inode_t *dir, int flags, int *links, char* lfile);
inode_t *symlink_follow(inode_t *lnk, int *links);
int inode_readlink(inode_t* ino, char* buf, int lg);
int inode_readuri(inode_t* ino, char* buf, int lg);


/* driver.c */
int driver_access(inode_t* ino);
int driver_release(inode_t* ino);

inode_t *inode_register(const char* name, inode_t* dir, stat_t *stat);





#endif  /* _SRC_VFS_H */
