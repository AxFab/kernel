#include <kernel/stream.h>
#include <kernel/memory.h>
#include <kernel/vfs.h>


// ===========================================================================
/** Read from a block or regular file
  */
ssize_t block_read (kStream_t *stm, void *buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;

  // Loop inside the buffer
  while (count > 0) {
    // Get the current page
    ssize_t poff = ALIGN_DW(stm->position_, PAGE_SIZE);
    inode_page (stm->ino_, poff, &page);
    void *address = (char *)mmu_temporary(&page) + (stm->position_ - poff);

    // Capacity ahead
    ssize_t cap = PAGE_SIZE - stm->position_ + poff;
    cap = MIN(cap, (ssize_t)(stm->ino_->stat_.length_ - stm->position_));
    cap = MIN (cap, (ssize_t)count);

    if (cap == 0)
      break;

    // Copy data
    memcpy (buf, address, cap);
    count -= cap;
    stm->position_ += cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  return bytes;
}


// ---------------------------------------------------------------------------
/** Write on a lock or regular file
  */
ssize_t block_write (kStream_t *stm, const void *buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;

  // Loop inside the buffer
  while (count > 0) {
    // Get the current page
    ssize_t poff = ALIGN_DW(stm->position_, PAGE_SIZE);
    inode_page (stm->ino_, poff, &page);
    void *address = (char *)mmu_temporary(&page) + (stm->position_ - poff);

    // Capacity ahead
    ssize_t cap = PAGE_SIZE - stm->position_ + poff;
    // cap = MIN(cap, stm->ino_->stat_.length_);
    // TODO Ask to file system before increase length
    cap = MIN (cap, (ssize_t)count);

    if (cap == 0)
      break;

    // Copy data
    memcpy (address, buf, cap);
    count -= cap;
    stm->position_ += cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  return bytes;
}


// ---------------------------------------------------------------------------
/** */
ssize_t dir_data (kStream_t *stm, void *buf, size_t count)
{
  __nounused(stm);
  __nounused(buf);
  __nounused(count);
  __seterrno(EISDIR);
  return -1;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
