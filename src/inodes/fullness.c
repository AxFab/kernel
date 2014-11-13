/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Routines to handle inode caching and fullness policy.
 */
#include <kernel/inodes.h>
#include <kernel/cpu.h>


// ===========================================================================
/** Function to called to grab an inodes */
int kfs_grab(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_inc_i32(&ino->readers_);
  return __noerror();
}

// ---------------------------------------------------------------------------
/** Function to release an inodes */
int kfs_release(kInode_t* ino)
{
  if (!ino)
    return __seterrno (EINVAL);

  atomic_dec_i32(&ino->readers_);
  return __noerror();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
