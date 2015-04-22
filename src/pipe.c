#include <smkos/kernel.h>
#include <smkos/core.h>



struct SMK_Event {
  clock_t clock_;
  int type_;
  int value_;
};


/* ----------------------------------------------------------------------- */
/**  */
static kPipe_t * fs_create_pipe(kInode_t *ino)
{
  int vmaRg = VMA_FILE | VMA_READ | VMA_WRITE;
  kPipe_t *pipe = KALLOC (kPipe_t);

  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  pipe->size_ = ALIGN_UP(ino->stat_.length_, PAGE_SIZE);
  pipe->mmap_ = area_map(kSYS.mspace_, pipe->size_, vmaRg);
  ino->pipe_ = pipe;
  return pipe;
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
  klock(&ino->lock_);
  if (!pipe)
    pipe = fs_create_pipe (ino);

  while (lg > 0) {
    if (pipe->rpen_ >= pipe->size_)
      pipe->rpen_ = 0;

    /* Get Address */
    address = (void*)(pipe->mmap_->address_ + pipe->wpen_);

    /* Capacity ahead */
    cap = pipe->size_ - pipe->rpen_;
    cap = MIN(cap, pipe->avail_);
    cap = MIN(cap, lg);
    if (cap == 0)
      break;

    /* Copy data */
    memcpy (buf, address, cap);
    lg -= cap;
    pipe->rpen_ += cap;
    pipe->avail_ -= cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  kunlock(&ino->lock_);
  return bytes;
}


/* ----------------------------------------------------------------------- */
/**  */
size_t fs_pipe_write(kInode_t *ino, const void* buf, size_t lg, int flags)
{
  size_t bytes = 0;
  size_t cap = 0;
  void* address;
  kPipe_t *pipe = ino->pipe_;

  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  /* Loop inside the buffer */
  klock(&ino->lock_);
  if (!pipe)
    pipe = fs_create_pipe (ino);

  if (flags) {
    if (pipe->avail_ < lg)
      return 0;
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

  kunlock(&ino->lock_);
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

  // if (ino->subSystem_ == NULL) {
    if (fs_pipe_write(ino, &event, sizeof(event), 1) == 0)
      return __seterrno(EAGAIN);
    return __seterrno(0);
  // }

  // return ENOSYS;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
