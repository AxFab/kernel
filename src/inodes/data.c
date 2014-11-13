/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used for common file operations.
 */
#include <kernel/inodes.h>
#include <kernel/memory.h>
#include <kernel/info.h>
#include <kernel/scheduler.h>


// ===========================================================================
/**
 */
int kfs_feed(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (ino != NULL && buffer != NULL);

  if (ino->fs_->read == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  if (KLOG_VFS) kprintf ("FS] Read '%s' on LBA %d using %x\n", ino->name_, offset, ino->fs_->read);
  MOD_ENTER;
  int err = ino->fs_->read (ino, buffer, length, offset);
  MOD_LEAVE;
  if (err) {
    __seterrno (err);
    return -1;
  }

  return length;
}


// ---------------------------------------------------------------------------
int kfs_sync(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (ino != NULL && buffer != NULL);

  if (ino->fs_->write == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  if (KLOG_VFS) kprintf ("FS] Write '%s' on LBA %d using %x\n", ino->name_, offset, ino->fs_->write);
  int err = ino->fs_->write (ino, buffer, length, offset);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return length;
}


// ---------------------------------------------------------------------------
/** Find a page that will correspond to the file page. */
int kfs_map (kInode_t*ino, off_t offset, uint32_t* page)
{
  int i;
  offset = ALIGN_DW (offset, PAGE_SIZE);
  klock (&ino->lock_, LOCK_FS_MAP);
  for (i = 0; i < ino->pageCount_; ++i) {
    if (ino->pagesCache_[i].offset_ == offset && ino->pagesCache_[i].phys_ != 0)
      break;
  }

  if (i == ino->pageCount_) {

    kPage_t* cache = kalloc (sizeof(kPage_t) * (ino->pageCount_ + 8));
    if (ino->pagesCache_) {
      memcpy (cache, ino->pagesCache_, sizeof(kPage_t) * ino->pageCount_);
      kfree(ino->pagesCache_);
    }

    ino->pageCount_ += 8;
    ino->pagesCache_ = cache;
    if (ino->fs_->map != NULL) {

      kunlock (&ino->lock_);
      MOD_ENTER;
      uint32_t spg = ino->fs_->map(ino, offset);
      MOD_LEAVE;
      klock (&ino->lock_, LOCK_FS_MAP);
      assert (cache[i].phys_ == 0); // FIXME not an assert.
      cache[i].phys_ = spg;
      if (cache[i].phys_ == 0)
        return __seterrno (EIO);

    } else {
      void* address = kpg_temp_page (&cache[i].phys_);
      kunlock (&ino->lock_);
      kfs_feed(ino, address, PAGE_SIZE / ino->stat_.block_, offset/ ino->stat_.block_);
      klock (&ino->lock_, LOCK_FS_MAP);
    }

    cache[i].offset_ = offset;
    cache[i].flags_ = 0;
  }

  // kprintf ("fs] map {P#%d}  [%d]->0x%x <%s:%d>\n", kCPU.current_->process_->pid_, i, ino->pagesCache_[i].phys_, ino->name_, offset);
  *page = ino->pagesCache_[i].phys_;
  kunlock (&ino->lock_);
  return __noerror();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
