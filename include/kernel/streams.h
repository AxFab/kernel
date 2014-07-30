#ifndef STREAMS_H__
#define STREAMS_H__

#include <kernel/core.h>

struct kStream
{
  kInode_t*     ino_;
  size_t        position_;
  int           flags_;
};

struct kFifoPen
{
  size_t producer_;
  size_t consumer_;
  // Add pending reading... (would block)
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
ssize_t kstm_read_stream (kStream_t* stream, void* buf, size_t length);
ssize_t kstm_write_stream (kStream_t* stream, void* buf, size_t length);
ssize_t kstm_read (int fd, void* buf, size_t length, off_t off);
ssize_t kstm_write (int fd, void* buf, size_t length, off_t off);
off_t kstm_seek(int fd, off_t offset, int whence);



#endif /* STREAMS_H__ */
