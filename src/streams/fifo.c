#include <kernel/streams.h>
#include <kernel/memory.h>
#include <kernel/vfs.h>
// #include <kernel/info.h>
// #include <kernel/scheduler.h>
// #include <kernel/vfs.h>
// #include <kernel/uparams.h>
// #include <stdio.h>


// ===========================================================================


// ---------------------------------------------------------------------------
/** Read from a FIFO
  */
ssize_t fifo_read (kInode_t* ino, void* buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;
  kFifo_t* fifo = ino->fifo_;
  
  // @todo search the next '\n'

  // Loop inside the buffer
  while (count > 0) {
    if (fifo->rpen_ >= fifo->size_)
      fifo->rpen_ = 0;

    // Get the current page
    // @todo, rename kpg_temp_page
    ssize_t poff = ALIGN_DW(fifo->rpen_, PAGE_SIZE);
    inode_page (ino, poff, &page);
    void* address = (char*)kpg_temp_page(&page) + (fifo->rpen_ - poff);

    // Capacity ahead
    ssize_t cap = PAGE_SIZE - fifo->rpen_ + poff;
    cap = MIN(cap, fifo->avail_);
    cap = MIN (cap, fifo->size_ - fifo->rpen_); 
    cap = MIN (cap, (ssize_t)count);
    if (cap == 0)
      break;

    // Copy data
    memcpy (buf, address, cap);
    count -= cap;
    fifo->rpen_ += cap;
    fifo->avail_ -= cap;
    bytes += cap;
    buf = ((char*)buf) + cap;
  }

  // Force rewind
  if (fifo->rpen_ >= fifo->size_)
    fifo->rpen_ = 0;

  return bytes;
}


// ---------------------------------------------------------------------------
/** Write on a FIFO
  */
ssize_t fifo_write (kInode_t* ino, const void* buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;
  kFifo_t* fifo = ino->fifo_;

  // Loop inside the buffer
  while (count > 0) {
    if (fifo->wpen_ >= fifo->size_)
      fifo->wpen_ = 0;

    // Get the current page
    // @todo, rename kpg_temp_page
    ssize_t poff = ALIGN_DW(fifo->wpen_, PAGE_SIZE);
    inode_page (ino, poff, &page);
    void* address = (char*)kpg_temp_page(&page) + (fifo->wpen_ - poff);

    // Capacity ahead
    ssize_t cap = PAGE_SIZE - fifo->wpen_ + poff;
    cap = MIN (cap, fifo->size_ - fifo->avail_);
    cap = MIN (cap, fifo->size_ - fifo->wpen_); 
    cap = MIN (cap, (ssize_t)count);
    if (cap == 0)
      break;

    // Copy data
    memcpy (address, buf, cap);
    count -= cap;
    fifo->wpen_ += cap;
    fifo->avail_ += cap;
    bytes += cap;
    buf = ((char*)buf) + cap;
  }

  // Force rewind
  if (fifo->wpen_ >= fifo->size_)
    fifo->wpen_ = 0;

  return bytes;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
