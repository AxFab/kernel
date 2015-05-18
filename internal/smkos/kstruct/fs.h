/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Structure and macros for file system module.
 */
#pragma once

#include <smkos/kernel.h>
#include <smkos/stat.h>

/* DEVICE
  *
  * The kernel keep on cache the entry available on various device mounted on
  * the system.
  */


/* ----------------------------------------------------------------------- */
/** Hold information about file system module.
  * Object are created by the function `register_driver` and
  * destroy on `unregister_driver`
  */
struct kDriver {
  const char *name_;

  int major_;
  struct llnode allNd_;
  atomic_t usage_;

  int (*mount)(kInode_t* dev, const char *name);
  int (*unmount)(kInode_t* dev, void* info);
  int (*dispose)();

  int (*map)(kInode_t *fp, size_t offset, page_t *page);
  int (*sync)(kInode_t *fp, size_t offset, page_t page);

  int (*lookup)(const char *name, kInode_t *dir, SMK_stat_t *file);
  int (*readdir)();
  int (*readlink)();
  int (*create)(const char *name, kInode_t *dir, int mode, size_t lg, SMK_stat_t *stat);

  int (*read)(kInode_t *fp, void *buffer, size_t length, size_t offset);
  int (*write)(kInode_t *fp, const void *buffer, size_t length, size_t offset);
  int (*flush)(kInode_t *fp);
};



/* ----------------------------------------------------------------------- */
struct kDevice {
  kInode_t* ino_;
  kDriver_t *fs_;
  kInode_t* underlyingDev_;
  struct llnode allNd_;
  struct mutex mutex_;
  void* data_;
  atomic_t usage_;
};


/* ----------------------------------------------------------------------- */
struct kInode {
  const char *name_;
  SMK_stat_t stat_;

  int readers_; /**< Counter for the number of place this inode is used. */
  kInode_t *parent_; /**< Parent inode */
  kInode_t *prev_;  /**< Previous inode */
  kInode_t *next_; /**< Next inode */
  kInode_t *child_; /**< First child inode */
  struct spinlock lock_; /**< Lock to control concurrent access. */
  struct llnode lruNd_; /**< Attach an unused inode on the LRU list */
  kDevice_t *dev_;
  struct bbtree pageTree_;

  kSubSystem_t *subsys_;
  union {
    kAssembly_t *assembly_;
    kPipe_t *pipe_;
  };
};


/* ----------------------------------------------------------------------- */
/** */
struct kPipe
{
  size_t rpen_; /**< Offset of the consumer(read) cursor */
  size_t wpen_; /**< Offset of the producer(write) cursor */
  size_t size_; /**< Total size of the buffer */
  size_t avail_; /**< Byte available to reading */
  // struct spinlock lock_;
  struct mutex mutex_;
  kMemArea_t* mmap_;
  int flags_;
  struct llhead waiting_;
};


struct kPage
{
  page_t phys_;
  size_t offset_;
  struct bbnode treeNd_;
};



#define FP_WRITE_FULL (1 << 3) /**< Ensure that write is full, usefulll for event structure or queue */
#define FP_BLOCK    (1 << 4)
#define FP_BY_LINE  (1 << 5)

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
