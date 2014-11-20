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

// ===========================================================================
//      Data Structures
// ===========================================================================
/** Structure holding stream informations */
struct kStream
{
  int           fd_;
  kInode_t*     ino_;
  size_t        position_;
  int           flags_;
};

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
//      Methods
// ===========================================================================

// STREAM/TERMIOS ============================================================
/**   */
ssize_t term_read(kInode_t* ino, void* buf, size_t count);
/**   */
ssize_t term_write(kInode_t* ino, const void* buf, size_t count);
/**   */
int term_event (kInode_t* ino, kEvent_t* event);
/**   */
kInode_t* term_create();


// STREAM/FIFO ===============================================================
/**   */
ssize_t fifo_read(kInode_t* ino, void* buf, size_t count);
/**   */
ssize_t fifo_write(kInode_t* ino, const void* buf, size_t count);
/**   */
kInode_t* fifo_create();


// STREAM/BLOCK ==============================================================
/**   */
ssize_t block_read(kInode_t* ino, void* buf, size_t count, ssize_t off);
/**   */
ssize_t block_write(kInode_t* ino, const void* buf, size_t count, ssize_t off);


// STREAM/FD =================================================================
/**   */
kStream_t* stream_open (int dir, const char* path, int mode, int flags);
/**   */
kStream_t* stream_create (int dir, const char* path, int mode, int flags);
/**   */
void stream_close (int fd);
/**   */
kStream_t* stream_set (kInode_t* ino, int flags);
/**   */
kStream_t* stream_get (int fd);


// STREAM/ASYNC ==============================================================
/**   */
int stream_wait_regist(kTask_t* task);
/**   */
int stream_wait_cancel(kTask_t* task);
/**   */
int stream_wait_trigger(long param1, long param2);



#endif /* KERNEL_STREAM_H__ */
