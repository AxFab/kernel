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
#include <kernel/vfs.h>
#include <kernel/params.h>
#include <kernel/memory.h>


// ---------------------------------------------------------------------------
/** Request the file system to feed the inode page buffer. 
  */
int feed_inode(kInode_t* ino, void* buffer, size_t length, off_t offset)
{
  assert (PARAM_KOBJECT (ino, kInode_t));
  // @todo assert (PARAM_KERNEL_BUFFER(buffer, length, PAGE_SIZE));
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
  if (((size_t)offset >= ino->stat_.length_ && ino->stat_.length_ != 0)) {
    __seterrno (EINVAL);
    return NULL;
  }

  kBucket_t* buck;
  klock (&ino->lock_);

  // Look on already cached buckets
  for_each (buck, &ino->buckets_, kBucket_t, node_) {
    if (buck->offset_ <= offset && 
          ((size_t)buck->offset_ + buck->length_) > (size_t)offset) {
      kunlock (&ino->lock_);
      return buck;
    }
  }

  // Look for the new page
  size_t length = PAGE_SIZE;
  page_t page = 0;
  if (ino->dev_->map != NULL) {
    length = PAGE_SIZE;
    offset = ALIGN_DW (offset, PAGE_SIZE);
    MODULE_ENTER(&ino->lock_, &ino->dev_->lock_);
    page = ino->dev_->map(ino, offset);
    MODULE_LEAVE(&ino->lock_, &ino->dev_->lock_);

  } else {
    // length = ino->stat_.block_;
    offset = ALIGN_DW (offset, PAGE_SIZE); // ino->stat_.block_);
    void* address = mmu_temporary (&page);
    if (feed_inode(ino, address, PAGE_SIZE / ino->stat_.block_, offset/ ino->stat_.block_))
      page = 0;
  }

  if (page == 0) {
    kunlock (&ino->lock_);
    __seterrno (EIO);
    return NULL;
  }

  // Create a new bucket
  // kprintf (LOG, "Create a new bucket\n");
  buck = KALLOC(kBucket_t);
  buck->offset_ = offset;
  buck->flags_ = 1;
  buck->length_ = length;
  buck->phys_ = page;
  klist_push_front(&ino->buckets_, &buck->node_);
  kunlock (&ino->lock_);
  return buck;
}



// ---------------------------------------------------------------------------
/** Find a physique page for the content of an inode. 
  * @note Legacy, should be replace progressively by inode_bucket.
  */
int inode_page(kInode_t* ino, off_t offset, page_t* page)
{
  kBucket_t* bucket = inode_bucket(ino, offset);
  if (bucket == NULL)
    return __geterrno();
  *page = bucket->phys_;
  return __seterrno(0);
}


// ---------------------------------------------------------------------------
/** Function to called to grab an inodes */
int inode_open (kInode_t* ino) 
{
  if (!ino)
    return __seterrno (EINVAL);

  // @todo be sure that the file is available
  klock (&ino->lock_);
  ++ino->readers_;
  klist_remove_if(&kSYS.inodeLru_, &ino->lruNd_);
  kunlock (&ino->lock_);
  return __noerror();
}

// ---------------------------------------------------------------------------
/** Function to release an inodes */
int inode_close (kInode_t* ino) 
{
  if (!ino)
    return __seterrno (EINVAL);

  // @todo if zero, push pression on scavenger
  klock (&ino->lock_);
  if (--ino->readers_ <= 0) {
    klist_remove_if(&kSYS.inodeLru_, &ino->lruNd_);
    klist_push_front(&kSYS.inodeLru_, &ino->lruNd_);
  }

  kunlock (&ino->lock_);
  return __noerror();

}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
