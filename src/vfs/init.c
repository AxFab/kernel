
#include <smkos/vfs.h>
#include <skc/stat.h>


extern dirent_t *allocat_dirent(const TCHAR *name, dirent_t *dir);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
struct vfs __VFS;

static void create_init_directory(dirent_t **ent, const TCHAR *name)
{
  *ent = drvfs_mknod(name, __VFS.root_, S_IFDIR | 0755);
  rw_wrunlock(&(*ent)->lock_);
  vfs_close(*ent);
}


static void vfs_unregister(dirent_t *ent) 
{

  if (ent->prev_ != NULL) 
    ent->prev_->next_ = ent->next_;
  else 
    ent->parent_->child_ = ent->next_;

  if (ent->next_ != NULL)
    ent->next_->prev_ = ent->prev_;

  /* Free all pages */
  if (ent->ino_ != NULL) {
    // TODO ino->rlink_-- < 0;
    drvfs_release(ent->ino_);
    --__VFS.inoCnt_;
  }

  /// @todo Free all buckets and stream objects...
  /// @todo Push to garbadge candidate
  /// @todo Free name
  /// @todo Free page cache first
  // if (ino->pagesCache_ != NULL) {
  //  kprintf ("We need to clean pages\n");
  // }

  --__VFS.entCnt_;
  kfree(ent->name_);
  kfree(ent->ino_);
  kfree(ent);
}


void vfs_init() 
{
  time_t now = time(NULL);
  __VFS.root_ = kalloc(sizeof(dirent_t));
  __VFS.root_->name_ = "";
  __VFS.root_->dev_ = kalloc(sizeof(device_t));
  __VFS.root_->dev_->inoCnt_ = 1;
  __VFS.root_->dev_->drv_ = kalloc(sizeof(driver_t));
  TMPFS_init(__VFS.root_->dev_->drv_); 
  __VFS.root_->ino_ = kalloc(sizeof(inode_t));
  __VFS.root_->ino_->mode_ = S_IFDIR | 0755;
  __VFS.root_->ino_->dev_ = __VFS.root_->dev_;
  __VFS.root_->ino_->atime_ = now;
  __VFS.root_->ino_->ctime_ = now;
  __VFS.root_->ino_->mtime_ = now;
  __VFS.root_->ino_->block_ = 1;

  __VFS.inoCnt_ = 1;
  __VFS.entCnt_ = 1;
  create_init_directory(&__VFS.dev_, "dev");
  create_init_directory(&__VFS.mnt_, "mnt");
  create_init_directory(&__VFS.sys_, "sys");
}


void vfs_mount_all()
{

}


void vfs_sweep() 
{
  vfs_scavenge(__VFS.inoCnt_);
  assert(__VFS.inoCnt_ == 1);
  kfree(__VFS.root_->ino_);
  kfree(__VFS.root_->dev_->drv_);
  kfree(__VFS.root_->dev_);
  kfree(__VFS.root_);
}

/* Call the inode scavenger which will try to free cached data */
void vfs_scavenge(int count)
{
  int suppr;
  dirent_t *itr;
  dirent_t *ent;
  while (count > 0) {
    itr = ll_first(&__VFS.lru_, dirent_t, lruNd_);
    if (itr == NULL)
      return;
    suppr = 0;
    while (itr == NULL && count > 0) {
      ent = itr;
      itr = ll_next(itr, dirent_t, lruNd_);
      rw_wrlock(&ent->parent_->lock_);
      rw_wrlock(&ent->lock_);
      assert(ent->readers_ == 0);
      if (ent->child_ != NULL) {
        rw_wrunlock(&ent->parent_->lock_);
        rw_wrunlock(&ent->lock_);
        continue;
      }

      vfs_unregister(ent);
      ++suppr;
    }

    if (suppr == 0)
      return;
  }
} 

void vfs_close(dirent_t *ent)
{
  rw_wrlock(&ent->lock_);
  if (--ent->readers_ <= 0)
    ll_push_back(&__VFS.lru_, &ent->lruNd_);
  rw_wrunlock(&ent->lock_);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

const char __ino_type[] = {
  '0', '-', 'd', 'l', 'b', 'c', 'p', '7', '8', '9',
};

const char *__ino_rights[] = {
  "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx", 
  "???", 
};

static void vfs_print(dirent_t *ent, int depth) 
{
  int i, j;
  dirent_t *wrk;
  inode_t *ino = ent->ino_;
  int mode = ino->mode_;

  kprintf("%c%s%s%s %2d ",
    __ino_type[mode>> 12],
    __ino_rights[(mode >> 6) & 07],
    __ino_rights[(mode >> 3) & 07],
    __ino_rights[(mode >> 0) & 07],
    ent->readers_);

    kprintf("  %#03x <%2d, %2d> %4d  %s ",ino->uid_, 
      ino->major_, ino->minor_, ino->block_, kpsize(ino->length_));

    for (i = 0; i < depth - 1; ++i) {
      j = i;
      wrk = ent;

      while (j++ < depth - 1)
        wrk = wrk->parent_;
      kprintf(wrk->next_ ? "    |" : "     ");
    }

    if (depth > 0) 
      kprintf(ent->next_ ? "    |--" : "    `--");
  
    kprintf(" %s\n", ent->name_);
}

void vfs_display() 
{
  int depth = 0;
  dirent_t *ent = __VFS.root_;

  while (ent != NULL) {
    vfs_print(ent, depth);
    if (ent->child_) {
      ++depth;
      ent = ent->child_;
    } else if (ent->next_) {
      ent = ent->next_;
    } else {
      while (ent != NULL && ent->next_ == NULL) {
        --depth;
        ent = ent->parent_;
      }
      if (ent != NULL)
        ent = ent->next_;
    }
  }
}

