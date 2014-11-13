/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used for data file operations.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Request the file system to feed the inode page buffer. 
  */
int feed_inode(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (PARAM_KOBJECT (ino, kInode_t));
  assert (PARAM_KERNEL_BUFFER(buffer, length, PAGE_SIZE));
  assert ((size_t)offset < ino->stat_.length_);
  assert (kislocked(&ino->lock_));

  if (ino->dev_->read == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  kprintf (LOG_VFS, "Read '%s' on LBA %d using %x\n", 
    ino->name_, offset, ksymbol(ino->dev_->read));

  MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
  int err = ino->dev_->read (ino, buffer, length, offset);
  MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return 0;
}


// ---------------------------------------------------------------------------
/** Request the file system to synchronize against the inode page buffer. 
  */
int sync_inode(kInode_t* ino, const void* buffer, size_t length, off_t offset)
{
  assert (PARAM_KOBJECT (ino, kInode_t));
  assert (PARAM_KERNEL_BUFFER(buffer, length, PAGE_SIZE));
  assert ((size_t)offset < ino->stat_.length_);
  assert (kislocked(&ino->lock_));

  if (ino->dev_->write == NULL) {
    __seterrno (EINVAL);
    return -1;
  }

  kprintf (LOG_VFS, "Write '%s' on LBA %d using %x\n", 
    ino->name_, offset, ksymbol(ino->dev_->write));

  MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
  int err = ino->dev_->write (ino, buffer, length, offset);
  MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);
  if (err) {
    __seterrno (err);
    return -1;
  }

  return 0;
}


// ---------------------------------------------------------------------------
/** Find a memory bucket for the content of an inode. 
  */
int inode_bucket(kInode_t*ino, off_t offset, uint32_t* page)
{
  __seterrno(ENOSYS);
  return -1;
}


// ---------------------------------------------------------------------------
/** Find a physique page for the content of an inode. 
  * @note Legacy, should be replace progressively by inode_bucket.
  */
int inode_page(kInode_t*ino, off_t offset, uint32_t* page)
{
  int i;
  offset = ALIGN_DW (offset, PAGE_SIZE);
  klock (&ino->lock_, LOCK_FS_MAP);
  for (i = 0; i < ino->pageCount_; ++i) {
    if (ino->pagesCache_[i].offset_ == offset && ino->pagesCache_[i].phys_ != 0)
      break;
  }

  if (i == ino->pageCount_) {

    kBucket_t* cache = kalloc (sizeof(kBucket_t) * (ino->pageCount_ + 8));
    if (ino->pagesCache_) {
      memcpy (cache, ino->pagesCache_, sizeof(kBucket_t) * ino->pageCount_);
      kfree(ino->pagesCache_);
    }

    ino->pageCount_ += 8;
    ino->pagesCache_ = cache;
    if (ino->dev_->map != NULL) {

      MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
      uint32_t spg = ino->dev_->map(ino, offset);
      MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);
      assert (cache[i].phys_ == 0); // FIXME not an assert.
      cache[i].phys_ = spg;
      if (cache[i].phys_ == 0)
        return __seterrno (EIO);

    } else {
      void* address = kpg_temp_page (&cache[i].phys_);
      kunlock (&ino->lock_);
      feed_inode(ino, address, PAGE_SIZE / ino->stat_.block_, offset/ ino->stat_.block_);
      klock (&ino->lock_, LOCK_FS_MAP);
    }

    cache[i].offset_ = offset;
    cache[i].flags_ = 0;
    cache[i].length_ = PAGE_SIZE;
  }

  // kprintf ("fs] map {P#%d}  [%d]->0x%x <%s:%d>\n", kCPU.current_->process_->pid_, i, ino->pagesCache_[i].phys_, ino->name_, offset);
  *page = ino->pagesCache_[i].phys_;
  kunlock (&ino->lock_);
  return __noerror();

}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
