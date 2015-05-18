#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/user.h>
#include <smkos/kstruct/task.h>
#include <smkos/event.h>


/* ----------------------------------------------------------------------- */
/**  */
kPipe_t * fs_create_pipe(kInode_t *ino)
{
  int vmaRg = VMA_FIFO | VMA_READ | VMA_WRITE;
  kPipe_t *pipe;

  klock(&ino->lock_);
  if (ino->pipe_) {
    kunlock(&ino->lock_);
    return ino->pipe_;
  }

  pipe = KALLOC (kPipe_t);
  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  if (ino->stat_.length_ == 0)
    ino->stat_.length_ = PAGE_SIZE;
  pipe->size_ = ALIGN_UP(ino->stat_.length_, PAGE_SIZE);
  klock(&kSYS.mspace_->lock_);
  pipe->mmap_ = area_map(kSYS.mspace_, pipe->size_, vmaRg);
  pipe->flags_ = FP_BLOCK | FP_BY_LINE;
  kunlock(&kSYS.mspace_->lock_);
  ino->pipe_ = pipe;

  kunlock(&ino->lock_);
  return pipe;
}


/* ----------------------------------------------------------------------- */
/**  */
void fs_pipe_destroy(kInode_t *ino)
{
  assert (kislocked(&ino->lock_));
  area_unmap(kSYS.mspace_, ino->pipe_->mmap_);
  kfree(ino->pipe_);
}

/* ----------------------------------------------------------------------- */
/**  */
static size_t fs_pipe_newline(kPipe_t *pipe)
{
  size_t i, cap;
  size_t max = pipe->size_ - pipe->rpen_;
  char* address = (char*)(pipe->mmap_->address_ + pipe->rpen_);

  cap = MIN(max, pipe->avail_);
  for (i = 0; i < cap; ++i) {
    if (address[i] == '\n')
      return i + 1;
  }

  if (max > pipe->avail_)
    return 0;

  address = (char*)(pipe->mmap_->address_);
  cap = MAX(0, pipe->avail_ - max);
  for (i = 0; i < cap; ++i) {
    if (address[i] == '\n')
      return i + max + 1;
  }

  return 0;
}

/* ----------------------------------------------------------------------- */
/**  */
int fs_pipe_read(kInode_t *ino, void* buf, size_t lg)
{
  size_t bytes = 0;
  size_t cap = 0;
  void* address;
  kPipe_t *pipe = ino->pipe_;

  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  /* Loop inside the buffer */
  if (!pipe)
    pipe = fs_create_pipe (ino);

  /// @todo Mutex on pipes
  mtx_lock(&pipe->mutex_);
  while (lg > 0) {
    if (pipe->rpen_ >= pipe->size_)
      pipe->rpen_ = 0;

    /* Get Address */
    address = (void*)(pipe->mmap_->address_ + pipe->rpen_);

    /* Capacity ahead */
    cap = pipe->size_ - pipe->rpen_;
    if (pipe->flags_ & FP_BY_LINE)
      cap = MIN(cap, fs_pipe_newline(pipe));
    else
      cap = MIN(cap, pipe->avail_);
    cap = MIN(cap, lg);
    if (cap == 0) {
      if (!(pipe->flags_ & FP_BLOCK) || bytes != 0)
        break;
      wait_for(&pipe->mutex_, WT_PIPE_READ, &pipe->waiting_);
      continue;
    }

    /* Copy data */
    memcpy (buf, address, cap);
    lg -= cap;
    pipe->rpen_ += cap;
    pipe->avail_ -= cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  mtx_unlock(&pipe->mutex_);
  return bytes;
}


/* ----------------------------------------------------------------------- */
/**  */
size_t fs_pipe_write(kInode_t *ino, const void* buf, size_t lg)
{
  size_t i;
  size_t bytes = 0;
  size_t cap = 0;
  void* address;
  kPipe_t *pipe = ino->pipe_;
  kWait_t * wait;
  kWait_t * iter;
  bool haveNl = false;

  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  /* Search for '\n' */
  if (pipe->flags_ & FP_BY_LINE) {
    for (i=0; i<lg; ++i)
      if (((char*)buf)[i] == '\n') {
        haveNl = true;
        break;
      }
  }

  /* Loop inside the buffer */
  if (!pipe)
    pipe = fs_create_pipe (ino);

  mtx_lock(&pipe->mutex_);
  if (pipe->flags_ & FP_WRITE_FULL) {
    if (pipe->size_ - pipe->avail_ < lg) {
      mtx_unlock(&pipe->mutex_);
      return 0;
    }
  }

  while (lg > 0) {
    if (pipe->wpen_ >= pipe->size_)
      pipe->wpen_ = 0;

    /* Get Address */
    address = (void*)(pipe->mmap_->address_ + pipe->wpen_);

    /* Capacity ahead */
    cap = pipe->size_ - pipe->wpen_;
    cap = MIN(cap, pipe->size_ - pipe->avail_);
    cap = MIN(cap, lg);
    if (cap == 0)
      break;

    /* Copy data */
    memcpy (address, buf, cap);
    lg -= cap;
    pipe->wpen_ += cap;
    pipe->avail_ += cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  // IF BLOCKED !
  if (haveNl) {
    iter = ll_first(&pipe->waiting_, kWait_t, lnd_);
    while (iter) {
      wait = iter;
      iter = ll_next(iter, kWait_t, lnd_);
      if (wait->reason_ == WT_PIPE_READ) {
        wait->reason_ = WT_HANDLED;
        sched_insert(kSYS.scheduler_, wait->thread_);
        // ll_remove(&pipe->waiting_, &wait->lnd_);
      }
    }
  }

  mtx_unlock(&pipe->mutex_);
  return bytes;
}


/* ----------------------------------------------------------------------- */
/**  */
int fs_event(kInode_t *ino, int type, int value)
{
  struct SMK_Event event;
  event.clock_ = clock();
  event.type_ = type;
  event.value_ = value;

  if (ino->subsys_) {
    ino->subsys_->event(type, value);
    return __seterrno(0);
  }

  kprintf ("[E%x,%d-%s]", type,value,ino->name_);

  // if (ino->subSystem_ == NULL) {
  if (fs_pipe_write(ino, &event, sizeof(event)) == 0)
    return __seterrno(EAGAIN);
  return __seterrno(0);
  // }

  // return ENOSYS;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
