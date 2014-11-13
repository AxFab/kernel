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
  // kprintf (LOG_VFS, "FEED_INODE %d, / %d - %d locks\n", offset, (int)ino->stat_.length_, klockcount());
  assert ((size_t)offset < ino->stat_.length_ || ino->stat_.length_ == 0);
  assert (kislocked(&ino->lock_));

  if (ino->dev_->read == NULL)
    return __seterrno (EINVAL);

  kprintf (LOG_VFS, "Read '%s' on LBA %d using %s()\n", 
    ino->name_, offset, ksymbol(ino->dev_->read));

  MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
  int err = ino->dev_->read (ino, buffer, length, offset);
  MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);
  // kprintf (LOG_VFS, "End reading...\n");
  return __seterrno (err);
}


// ---------------------------------------------------------------------------
/** Request the file system to synchronize against the inode page buffer. 
  */
int sync_inode(kInode_t* ino, const void* buffer, size_t length, off_t offset)
{
  assert (PARAM_KOBJECT (ino, kInode_t));
  assert (PARAM_KERNEL_BUFFER(buffer, length, PAGE_SIZE));
  assert ((size_t)offset < ino->stat_.length_ || ino->stat_.length_ == 0);
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
kBucket_t* inode_bucket(kInode_t* ino, off_t offset)
{
  assert (PARAM_KOBJECT (ino, kInode_t));
  assert ((size_t)offset < ino->stat_.length_ || ino->stat_.length_ == 0);

  // klock (&ino->lock_);

  // // Look on already cached buckets
  // kBucket_t* buck = ino->buckets.f_;
  // while (buck != NULL) {
  //   if (buck->offset_ == offset) {
  //     return buck;
  //   }

  //   buck = buck->l_;
  // };


  // // @todo Search physique pages !

  // buck = KALLOC(kBucket_t);
  // buck->offset_ = offset;
  // buck->flags_ = 1;
  // buck->length_ = PAGE_SIZE;

  // // Attach at the end
  // buck->file_.l_ = NULL;
  // buck->file_.f_ = ino->buckets.l_;
  // ino->buckets.l_->l_ = buck;
  // ino->buckets.l_ = buck;

  // // Attach at the begining
  // buck->file_.f_ = NULL;
  // buck->file_.l_ =  ino->buckets_.f_;
  // if (ino->buckets_.f_) ino->buckets_.f_->l_;

  __seterrno(ENOSYS);
  return NULL;
}


// ---------------------------------------------------------------------------
/** Find a physique page for the content of an inode. 
  * @note Legacy, should be replace progressively by inode_bucket.
  */
int inode_page(kInode_t* ino, off_t offset, uint32_t* page)
{
  // kprintf (LOG_VFS, "Map '%s' on LBA %d using %s()\n", ino->name_, offset);
  assert (PARAM_KOBJECT (ino, kInode_t));
  assert ((size_t)offset < ino->stat_.length_ || ino->stat_.length_ == 0);

  klock (&ino->lock_);

  int i;
  int fr = -1;
  kBucket_t* cache = ino->pagesCache_;
  offset = ALIGN_DW (offset, PAGE_SIZE);

  // Look on already cached buckets
  for (i = 0; i < ino->pageCount_; ++i) {
    if (cache[i].offset_ == offset && cache[i].phys_ != 0) {
      *page = cache[i].phys_;
      kunlock (&ino->lock_);
      return 0;
    } else if (fr < 0 && cache[i].flags_ == 0) {
      fr = i;
    }
  }

  if (fr < 0) {
    // Allocate a new bucket array
    fr = ino->pageCount_;
    cache = kalloc (sizeof(kBucket_t) * (ino->pageCount_ + 8));
    if (ino->pagesCache_) {
      memcpy (cache, ino->pagesCache_, sizeof(kBucket_t) * ino->pageCount_);
      kfree(ino->pagesCache_);
    }

    ino->pageCount_ += 8;
    ino->pagesCache_ = cache;
  }

  assert (cache[fr].phys_ == 0 && cache[fr].flags_ == 0);

  if (ino->dev_->map != NULL) {

    MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
    uint32_t spg = ino->dev_->map(ino, offset);
    MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);

    cache[fr].phys_ = spg;
    if (cache[fr].phys_ == 0) {
      kunlock (&ino->lock_);
      return __seterrno (EIO);
    }

  } else {
    void* address = kpg_temp_page (&cache[fr].phys_);
    feed_inode(ino, address, PAGE_SIZE / ino->stat_.block_, offset/ ino->stat_.block_);
    // kprintf (LOG_VFS, "Map complete the feed...\n");
  }

  cache[fr].offset_ = offset;
  cache[fr].flags_ = 1;
  cache[fr].length_ = PAGE_SIZE;

  *page = cache[fr].phys_;
  kunlock (&ino->lock_);
  return 0;

}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
