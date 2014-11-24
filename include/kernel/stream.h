/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Header file of the stream module
 *      Build stream for process and exchange binary data between inodes.
 */
#ifndef KERNEL_STREAM_H__
#define KERNEL_STREAM_H__

#include <kernel/core.h>

// ===========================================================================
//      Definitions and Macros
// ===========================================================================
#define SEEK_SET   1
#define SEEK_CUR   2
#define SEEK_END   3

// ===========================================================================
//      Data Structures
// ===========================================================================
/** Structure holding stream informations */
struct kStream
{
  int           fd_;
  spinlock_t    lock_;
  kInode_t*     ino_;
  size_t        position_;
  int           flags_;
  ssize_t (*read)(kStream_t*, void*, size_t);
  ssize_t (*write)(kStream_t*, const void*, size_t);
};



// ===========================================================================
//      Methods
// ===========================================================================

// STREAM/TERMIOS ============================================================
/**   */
ssize_t term_read(kStream_t* stm, void* buf, size_t count);
/**   */
ssize_t term_write(kStream_t* stm, const void* buf, size_t count);
/**   */
int term_event (kStream_t* stm, kEvent_t* event);
/**   */
kInode_t* term_create();


// STREAM/FIFO ===============================================================
/**   */
ssize_t fifo_read(kStream_t* stm, void* buf, size_t count);
/**   */
ssize_t fifo_write(kStream_t* stm, const void* buf, size_t count);
/**   */
kInode_t* fifo_create();


// STREAM/BLOCK ==============================================================
/**   */
ssize_t block_read(kStream_t* stm, void* buf, size_t count);
/**   */
ssize_t block_write(kStream_t* stm, const void* buf, size_t count);
/**   */
ssize_t dir_data (kStream_t* stm, void* buf, size_t count);

// STREAM/FD =================================================================
/**   */
kStream_t* stream_open (int dir, const char* path, int mode, int flags);
/**   */
kStream_t* stream_create (int dir, const char* path, int mode, int flags);
/**   */
int stream_close (int fd);
/**   */
kStream_t* stream_set (kInode_t* ino, int flags);
/**   */
kStream_t* stream_get (int fd, int rights);
/** */
int stream_tty (kProcess_t* proc, kInode_t* tty);

// STREAM/DATA ===============================================================
/** */
ssize_t stream_read (int fd, void* buf, size_t length);
/** */
ssize_t stream_write (int fd, const void* buf, size_t length);
/** */
off_t stream_seek(int fd, off_t offset, int whence);


#endif /* KERNEL_STREAM_H__ */
