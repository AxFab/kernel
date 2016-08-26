#include <smkos/vfs.h>

static void drvfs_open(driver_t *drv)
{
  mtx_lock(&drv->mtx_);
}

static void drvfs_close(driver_t *drv)
{
  mtx_unlock(&drv->mtx_);
}


/* Allocate a new directory entry */
static dirent_t *allocat_dirent(const TCHAR *name, dirent_t *dir, inode_t *ino)
{
  int cmp;
  driver_t *drv = dir->dev_->drv_;
  dirent_t *wrk;
  dirent_t *ent = kalloc(sizeof(dirent_t));
  rw_wrlock(&ent->lock_);
  ent->parent_ = dir;
  ent->dev_ = dir->dev_;

  rw_wrlock(&dir->lock_);
  if (dir->child_ == NULL) {
    dir->child_ = ent;
  } else if (drv->compare(name, dir->child_->name_) < 0) {
    ent->next_ = dir->child_;
    dir->child_->prev_ = ent;
    dir->child_ = ent;
  } else {
    wrk = dir->child_;
    for (;;) {
      if (wrk->next_ == NULL) {
        ent->prev_ = wrk;
        wrk->next_ = ent;
        break;
      } 

      cmp = drv->compare(name, wrk->next_->name_);
      if (cmp > 0 && wrk->next_ != NULL) {
        wrk = wrk->next_;
        continue;
      } else if (cmp == 0) {
        kprintf(CSL_RED "WARN] dirent already exist." CSL_RESET "\n");
        rw_wrunlock(&ent->lock_);
        rw_wrunlock(&dir->lock_);
        kfree(ent);
        return NULL;
      } 

      ent->prev_ = wrk;
      ent->next_ = wrk->next_;
      wrk->next_->prev_ = ent;
      wrk->next_ = ent;
      break;
    }
  }

  ent->name_ = tcsdup(name);
  ent->ino_ = ino;
  ent->readers_ = 1;
  ++__VFS.inoCnt_;
  ++__VFS.entCnt_;
  rw_wrunlock(&dir->lock_);
  return ent;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


/* Refresh inode info (used for distributed data) */
int drvfs_refresh(inode_t *ino); 
/* Free inode-related driver meta-data */
int drvfs_release(inode_t *ino)
{
  int err = 0;
  // driver_t *drv = ino->dev_->drv_;
  // drvfs_open(drv);
  // err = drv->release(ino);
  // drvfs_close(drv);
  return err;
}

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
dirent_t* drvfs_lookup(const TCHAR *name, dirent_t* dir)
{
  int err; 
  dirent_t *ent;
  inode_t *ino = kalloc(sizeof(inode_t));
  driver_t *drv = dir->dev_->drv_;
  drvfs_open(drv);
  err = drv->lookup(name, dir->ino_, ino);
  drvfs_close(drv);
  if (err != 0) {
    kfree(ino);
    return NULL;
  }

  ent = allocat_dirent(name, dir, ino);
  return ent;
}


/* Create a new inode */
dirent_t* drvfs_mknod(const TCHAR *name, dirent_t* dir, int mode)
{
  int err;
  time_t now = time(NULL);
  dirent_t *ent;
  driver_t *drv = dir->dev_->drv_;
  inode_t *ino = kalloc(sizeof(inode_t));
  ino->mode_ = mode;
  ino->atime_ = now;
  ino->ctime_ = now;
  ino->mtime_ = now;
  drvfs_open(drv);
  err = drv->mknod(name, dir->ino_, ino);
  drvfs_close(drv);
  __seterrno(err);
  if (err != 0) {
    kfree(ino);
    return NULL;  
  }

  ent = allocat_dirent(name, dir, ino); 
  return ent;
}

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
/* Create a record type of inode */
int drvfs_mkrcd(const TCHAR *name, inode_t* dir, inode_t *ino, const void *buf, int lg);
/* */
int drvfs_getrcd(inode_t *ino, const TCHAR *buf, size_t lg);





int drvfs_readlink(inode_t *ino, TCHAR *buf, int lg)
{
  int err = 0;
  // driver_t *drv = ino->dev_->drv_;
  // drvfs_open(drv);
  // err = drv->readlink(ino, buf, lg);
  // drvfs_close(drv);
  return err;
}
