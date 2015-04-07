/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to read path names and browse directories.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Read an inode full pathname. */
ssize_t read_pathname(kInode_t *ino, int method, char *buf, int size)
{
  __seterrno(ENOSYS);
  return -1;
}


// ---------------------------------------------------------------------------
/** Read the content of a directory. */
int read_directory(kInode_t *dir, off_t offset, void *buf, size_t size)
{
  __seterrno(ENOSYS);
  return -1;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
