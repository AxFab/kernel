#ifndef STREAMS_H__
#define STREAMS_H__

#include <kernel/core.h>

struct kStream
{
  kInode_t*     ino_;
  size_t        position_;
  int           flags_;
};

struct kStreamType
{
  ssize_t (*read)(kInode_t* ino, void* buf, size_t count, off_t offset);
  ssize_t (*write)(kInode_t* ino, const void* buf, size_t count, off_t offset);
  // sync
};

struct kPipe
{
  ssize_t  outPen_;
  ssize_t  inPen_;
  ssize_t  length_;

  size_t producer_;
  size_t consumer_;
  // Add pending reading... (would block)
};


struct kFifo
{
  ssize_t rpen_;      ///< Offset of the consumer(read) cursor
  ssize_t wpen_;      ///< Offset of the producer(write) cursor
  ssize_t size_;      ///< Total size of the buffers
  ssize_t avail_;     ///< Byte available to reading
};



struct kFifoPen
{
  size_t producer_;
  size_t consumer_;
  // Add pending reading... (would block)
};

struct kNTty
{
  int         width_;
  int         height_;
  uint32_t    txColor_;
  uint32_t    bgColor_;
  uint32_t*   pixels_;

  int         row_;
  kStream_t*  input_;
  kStream_t*  output_;
  kLine_t*    first_;
  kLine_t*    top_;
  kLine_t*    last_;
  int         maxRow_;
  int         maxCol_;
  int         newInput_;
  int         isInput_;
  int         reEcho_;
};


struct kLine
{
  int         offset_;
  int         flags_;
  uint32_t    txColor_;
  uint32_t    bgColor_;
  kLine_t*    next_;
  kLine_t*    prev_;
};


struct kFrameBuffer
{
  int         width_;
  int         height_;
  uint32_t    txColor_;
  uint32_t    bgColor_;
  uint32_t*   pixels_;
};


kStream_t* kstm_get_fd (int fd, int mode);
int kstm_set_fd (kInode_t* ino, int flags);
int kstm_clear_fd (int fd);
int kstm_open(int dirfd, const char *path, int flags, mode_t mode);
int kstm_close (int fd);

int kstm_create(kInode_t* dir, const char *path, int flags, mode_t mode);
int kstm_pipe(int flags, mode_t mode, size_t length);

ssize_t kstm_read_block (kStream_t* stream, void* buf, size_t length, off_t off);
ssize_t kstm_write_block (kStream_t* stream, void* buf, size_t length, off_t off);

ssize_t kstm_read_pipe (kStream_t* stream, void* buf, size_t length);
ssize_t kstm_write_pipe (kStream_t* stream, void* buf, size_t length);
ssize_t kstm_read_pipe_line  (kStream_t* stream, void* buf, size_t length);
ssize_t kstm_available_data_pipe (kStream_t* stream);
kStream_t* kstm_create_pipe (int flags, size_t length);

// ssize_t kstm_read_tty (kStream_t* stream, void* buf, size_t length);
// ssize_t kstm_write_tty (kStream_t* stream, void* buf, size_t length);

ssize_t kstm_read (int fd, void* buf, size_t length, off_t off);
ssize_t kstm_write (int fd, void* buf, size_t length, off_t off);
off_t kstm_seek(int fd, off_t offset, int whence);



#endif /* STREAMS_H__ */
