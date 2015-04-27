#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/event.h>


/* ----------------------------------------------------------------------- */
/**  */
static kPipe_t * fs_create_pipe(kInode_t *ino)
{
  int vmaRg = VMA_FIFO | VMA_READ | VMA_WRITE;
  kPipe_t *pipe;

  klock(&ino->lock_);
  pipe = KALLOC (kPipe_t);

  assert (S_ISFIFO(ino->stat_.mode_) || S_ISCHR(ino->stat_.mode_));

  if (ino->stat_.length_ == 0)
    ino->stat_.length_ = PAGE_SIZE;
  pipe->size_ = ALIGN_UP(ino->stat_.length_, PAGE_SIZE);
  klock(&kSYS.mspace_->lock_);
  pipe->mmap_ = area_map(kSYS.mspace_, pipe->size_, vmaRg);
  kunlock(&kSYS.mspace_->lock_);
  ino->pipe_ = pipe;

  kunlock(&ino->lock_);
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
  if (!pipe)
    pipe = fs_create_pipe (ino);

  // TODO Mutex on pipes
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
  if (!pipe)
    pipe = fs_create_pipe (ino);

  // TODO Mutex on pipes
  if (flags) {
    if (pipe->size_ - pipe->avail_ < lg)
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

  kprintf ("[E%x,%d-%s]", type,value,ino->name_);

  // if (ino->subSystem_ == NULL) {
  if (fs_pipe_write(ino, &event, sizeof(event), 1) == 0)
    return __seterrno(EAGAIN);
  return __seterrno(0);
  // }

  // return ENOSYS;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
