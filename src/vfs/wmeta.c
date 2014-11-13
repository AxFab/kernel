/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to handle devices operations.
 */
#include "kernel/vfs.h"
#include "kernel/params.h"


// ---------------------------------------------------------------------------
/** Request the file system for the creation of a new inode. */
kInode_t* create_inode(const char* name, kInode_t* dir, mode_t mode, dev_t dev)
{

}


// ---------------------------------------------------------------------------
/** Request the file system to remove an inode. */
int remove_inode(kInode_t* ino)
{

}


// ---------------------------------------------------------------------------
/** Request the file system to update the inode metadata. */
int chmeta_inode(kInode_t* ino, kStat_t* stat)
{

}


// ---------------------------------------------------------------------------
/** Request the file system to change the path of the inode. */
int rename_inode(kInode_t* ino, const char* name, kInode_t* dir)
{

}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
