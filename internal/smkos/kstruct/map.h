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
 *      Structure and macros for memory mapping module.
 */
#pragma once

#include <smkos/kernel.h>


/* ----------------------------------------------------------------------- */
#define VMA_WRITE         0x002 /**< Pages can be written to */
#define VMA_EXEC          0x001 /**< Pages can be executed */
#define VMA_READ          0x004 /**< Pages can be read from */

#define VMA_KERNEL       0x10000

#define VMA_SHARED        0x008 /**< Page are shared */
#define VMA_MAYREAD       0x010 /**< The VMA_READ fSlag can be set */
#define VMA_MAYWRITE      0x020 /**< The VMA_WRITE flag can be set */
#define VMA_MAYEXEC       0x040 /**< The VMA_EXEC flag can be set */
#define VMA_MAYSHARED     0x080 /**< The VMA_SHARED flag can be set */
#define VMA_GROWSDOWN     0x100 /**< The area can grow downward */
#define VMA_GROWSUP       0x200 /**< The area can grow upward */

#define VMA_SHM           0x400 /**< The area is used for shared memory */
#define VMA_FILE          0x800 /**< The area map an executable file */
#define VMA_HEAP          0x1000 /**< The area map a process heap */
#define VMA_STACK         0x2000 /**< The area map a thread stack */
#define VMA_FIFO          0x4000

#define VMA_CODE          0x10000
#define VMA_DATA          0x20000

#define VMA_ACCESS      (VMA_WRITE | VMA_EXEC | VMA_READ)
#define VMA_MMU         (VMA_ACCESS | VMA_KERNEL)
#define VMA_ASSEMBLY    (VMA_CODE | VMA_DATA)
#define VMA_TYPE        (VMA_SHM | VMA_FILE | VMA_HEAP | VMA_STACK | VMA_FIFO)


/* ----------------------------------------------------------------------- */
#define PF_PROT   (1<<0)
#define PF_WRITE  (1<<1)
#define PF_USER   (1<<2)
#define PF_RSVD   (1<<3)
#define PF_INSTR  (1<<4)
#define PF_KERN   (1<<8)


/* ----------------------------------------------------------------------- */
struct kMemArea {
  size_t address_;
  size_t offset_;
  size_t limit_;
  kInode_t *ino_;
  atomic_t usage_;
  int flags_;

  kMemArea_t *prev_;
  kMemArea_t *next_;
  struct bbnode bbNode_;
  const char* at_;
};


/* ----------------------------------------------------------------------- */
struct kMemSpace {
  size_t phyPages_;
  size_t vrtPages_;
  struct spinlock lock_;

  kMemArea_t *first_;
  kMemArea_t *last_;
  struct bbtree bbTree_;

  size_t base_;
  size_t limit_;
};


/* ----------------------------------------------------------------------- */
struct kAssembly {
  size_t entryPoint_;
  kInode_t* ino_;
  struct llhead sections_;
  atomic_t usage_;
};


/* ----------------------------------------------------------------------- */
struct kSection {
  size_t address_;
  size_t length_;
  size_t align_;
  size_t offset_;
  struct llnode node_;
  int flags_;
};

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
