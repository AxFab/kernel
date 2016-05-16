#include <skc/fifo.h>
#include <skc/locks.h>
#include <stdlib.h>
#include <string.h>


#define malloc kalloc
void *kalloc(size_t);

struct fifo 
{
  size_t rpen_;
  size_t wpen_;
  size_t avail_;
  size_t size_;
  void *buf_;
  mutex_t mtx_;
  void (*wait)(fifo_t*, int);
  void (*awake)(fifo_t*, int);
};


/* Instanciate a new FIFO */
fifo_t *fifo_init(void *buf, size_t lg) 
{
  fifo_t *fifo = (fifo_t*)malloc(sizeof(fifo_t));
  memset(fifo, 0, sizeof(fifo_t));
  fifo->buf_ = buf;
  fifo->size_ = lg;
  return fifo;
}


/* Look for a specific bytes in consumable data */
size_t fifo_indexof(fifo_t *fifo, char ch) 
{
  size_t i;
  size_t cap;
  size_t max = fifo->size_ - fifo->rpen_;
  char *address = (char*)(fifo->buf_ + fifo->rpen_);

  cap = MIN(max, fifo->avail_);
  for (i = 0; i < cap; ++i)
    if (address[i] == ch)
      return i + 1;

  if (max > fifo->avail_)
    return 0;

  address = (char*)fifo->buf_;
  cap = (size_t)MAX(0, (int)fifo->avail_ - (int)max);
  for (i = 0; i < cap; ++i)
    if (address[i] == ch)
      return i + 1;

  return 0;
}


/* Read some bytes from the FIFO */
size_t fifo_out(fifo_t *fifo, void *buf, size_t lg, int flags)
{
  size_t cap = 0;
  size_t bytes = 0;
  void *address;

  mtx_lock(&fifo->mtx_);
  while (lg > 0) {
    if (fifo->rpen_ >= fifo->size_)
      fifo->rpen_ = 0;
    
    address = (void*)(fifo->buf_ + fifo->rpen_);
    cap = fifo->size_ - fifo->rpen_;
    cap = MIN(cap, fifo->avail_);
    if (flags & FP_EOL)
      cap = MIN(cap, fifo_indexof(fifo, '\n'));
    cap = MIN(cap, lg);
    if (cap == 0) {
      if (flags & FP_NOBLOCK || bytes != 0)
        break;
      if (fifo->wait)
        fifo->wait(fifo, flags);
      continue;
    }
    
    memcpy(buf, address, cap);
    fifo->rpen_ += cap;
    fifo->avail_ -= cap;
    lg -= cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  if (fifo->awake)
    fifo->awake(fifo, flags);
  mtx_unlock(&fifo->mtx_);
  return bytes;
}


/* Write some bytes on the FIFO */
size_t fifo_in(fifo_t *fifo, const void *buf, size_t lg, int flags)
{
  size_t cap = 0;
  size_t bytes = 0;
  void *address;

  mtx_lock(&fifo->mtx_);
  while (lg > 0) {
    if (fifo->wpen_ >= fifo->size_)
      fifo->wpen_ = 0;

    address = (void*)(fifo->buf_ + fifo->wpen_);
    cap = fifo->size_ - fifo->wpen_;
    cap = MIN(cap, fifo->size_ - fifo->avail_);  
    cap = MIN(cap, lg);
    if (cap == 0) {
      if (flags & FP_NOBLOCK || bytes != 0)
        break;
      if (fifo->wait)
        fifo->wait(fifo, flags);
      continue;
    }
    
    memcpy(address, buf, cap);
    fifo->wpen_ += cap;
    fifo->avail_ += cap; 
    lg -= cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  if (fifo->awake)
    fifo->awake(fifo, flags);
  mtx_unlock(&fifo->mtx_);
  return bytes;
}


/* Reinitialize the queue */
void fifo_reset(fifo_t *fifo)
{
  mtx_lock(&fifo->mtx_);
  fifo->rpen_ = 0;
  fifo->wpen_ = 0;
  fifo->avail_ = 0;
  mtx_unlock(&fifo->mtx_);
}

