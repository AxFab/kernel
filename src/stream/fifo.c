#include <kernel/memory.h>
#include <kernel/stream.h>
#include <kernel/vfs.h>
#include <kernel/info.h>

#define FIFO_SIZE     (3 * PAGE_SIZE)

// ---------------------------------------------------------------------------
/** Structure holding fifo cursor. */
struct kFifo
{
  ssize_t rpen_;      ///< Offset of the consumer(read) cursor
  ssize_t wpen_;      ///< Offset of the producer(write) cursor
  ssize_t size_;      ///< Total size of the buffers
  ssize_t avail_;     ///< Byte available to reading
};


// ===========================================================================
/** Read from a FIFO
  */
ssize_t fifo_read (kStream_t* stm, void* buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;
  kFifo_t* fifo = stm->ino_->fifo_;
  
  // @todo search the next '\n'

  // Loop inside the buffer
  while (count > 0) {
    if (fifo->rpen_ >= fifo->size_)
      fifo->rpen_ = 0;

    // Get the current page
    // @todo, rename mmu_temporary
    ssize_t poff = ALIGN_DW(fifo->rpen_, PAGE_SIZE);
    inode_page (stm->ino_, poff, &page);
    void* address = (char*)mmu_temporary(&page) + (fifo->rpen_ - poff);

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
ssize_t fifo_write (kStream_t* stm, const void* buf, size_t count)
{
  uint32_t page;
  ssize_t bytes = 0;
  kFifo_t* fifo = stm->ino_->fifo_;

  // Loop inside the buffer
  while (count > 0) {
    if (fifo->wpen_ >= fifo->size_)
      fifo->wpen_ = 0;

    // Get the current page
    // @todo, rename mmu_temporary
    ssize_t poff = ALIGN_DW(fifo->wpen_, PAGE_SIZE);
    inode_page (stm->ino_, poff, &page);
    void* address = (char*)mmu_temporary(&page) + (fifo->wpen_ - poff);

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
/** */
kInode_t* fifo_create()
{
  static int auto_incr = 0;
  char no[10];

  snprintf (no, 10, "pi%d", auto_incr++);
  kInode_t* ino = create_inode(no, kSYS.pipeNd_, S_IFIFO | 0600, FIFO_SIZE);
  assert (ino != NULL);

  kFifo_t* fifo = KALLOC(kFifo_t);
  fifo->size_ = FIFO_SIZE;
  ino->fifo_ = fifo;

  return ino;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
