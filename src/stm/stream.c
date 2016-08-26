#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/sysapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>
#include <smkos/file.h>
#include <smkos/fcntl.h>

kInode_t *search_child2(const char *name, kInode_t *dir);
void term_write (kTerm_t *term);

ssize_t stream_read(kInode_t* ino, void *data, size_t lg, off_t off, int flags)
{
  ssize_t bytes;
  switch (ino->stat_.mode_ & S_IFMT) {
  case S_IFBLK:
  case S_IFREG:
    return fs_reg_read(ino, data, lg, off);

  case S_IFTTY:
    ino = search_child2(".in", ino);
    assert(ino != NULL);
    bytes = fs_pipe_read(ino, data, lg);
    return bytes;

  case S_IFCHR:
  case S_IFIFO:
    return fs_pipe_read(ino, data, lg);

  case S_IFDIR:
    __seterrno(EISDIR);
    return -1;

  default:
    __seterrno(EBADF);
    return -1;
  }
}


ssize_t stream_write(kInode_t* ino, const void *data, size_t lg, off_t off, int flags)
{
  ssize_t bytes;
  switch (ino->stat_.mode_ & S_IFMT) {
  case S_IFBLK:
  case S_IFREG:
    //return fs_reg_write(ino_, data, lg, off);
    return -1;

  case S_IFTTY:
    ino = search_child2(".out", ino);
    assert(ino != NULL);
    bytes = fs_pipe_write(ino, data, lg);
    term_write(NULL);
    return bytes;

  case S_IFCHR:
  case S_IFIFO:
    return fs_pipe_write(ino, data, lg);

  case S_IFDIR:
    __seterrno(EISDIR);
    return -1;

  default:
    __seterrno(EBADF);
    return -1;
  }
}


int stream_flush(kInode_t* ino, int flags)
{
  return __seterrno(EAGAIN);
}

// int stream_event(kInode_t* ino, int type, int value)
// {
//   int ret;
//   kEvent_t event;
//   event.clock_ = clock();
//   event.type_ = type;
//   event.value_ = value;

//   switch (ino->stat_.mode_ & S_IFMT) {
//   case S_IFCHR:
//   case S_IFIFO:
//     ret = (int)fs_pipe_write(ino, &event, sizeof(event));
//     if (ret == 0)
//       return __seterrno(EAGAIN);
//     if (ret != sizeof(event))
//       return __seterrno(EIO);
//     return __seterrno(0);
//   default:
//     return __seterrno(EBADF);
//   }
// }


