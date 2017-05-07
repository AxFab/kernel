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
 *      Virtual memory area managment.
 */
#include <klib/memory.h>
#include <errno.h>


void Memory_bound(memspace_t *mspace, int type) {
  mspace->lower_ = 4 * _Mib_;
  mspace->upper_ = 2 * _Gib_;
}

memspace_t *Memory_create() {
  memspace_t *mspace;
  mspace = KALLOC(memspace_t);
  Memory_bound(mspace, MEMSP_USERSPACE);
  mspace->flags_ = MEMSP_USERSPACE;
  return mspace;
}

/* Append a new virtual memory area using specified parameter.
  In case address is null, the first available area is allocated if not either
  the exact address is provided or ENOMEM is returned.
  */
int Memory_append(memspace_t *mspace, void* address, size_t length, inode_t *ino, off_t offset, int flags, memvmap_t **pVma) {
  memvmap_t *vma;
  memvmap_t *cursor;
  memvmap_t *next;
  size_t bound = mspace->lower_;
  length = ALIGN_UP(length, PAGE_SIZE);
  if (length == 0 || length > MEM_VMA_LENGTH) {
    return EINVAL;
  }


  if (address != NULL) {
    cursor = bb_search_less(&mspace->mmaps_, (size_t)address, memvmap_t, node_);
  } else {
    cursor = bb_full_left(mspace->mmaps_.root_, memvmap_t, node_);
  }

  if (cursor == NULL) {
    if (address == NULL) {
      address = (void*)bound;
    } else {
      cursor = bb_full_left(mspace->mmaps_.root_, memvmap_t, node_);
      if (cursor != NULL) {
        bound = cursor->node_.value_;
      }
    }

    if ((size_t)address + length < bound) {
      return ENOMEM;
    }
  } else {
    for (;;) {
      next = bb_next(&cursor->node_, memvmap_t, node_);
      if (next == NULL || cursor->node_.value_ + cursor->length_ + length <= next->node_.value_) {
        break;
      }
    }
    // bound = next->node_.value_;
    if (next == NULL && cursor->node_.value_ + cursor->length_ + length > mspace->upper_) {
      return ENOMEM;
    }
    address = (void*)(cursor->node_.value_ + cursor->length_);
  }

  vma = KALLOC(memvmap_t);
  vma->length_ = length;
  vma->ino_ = ino;
  vma->off_ = offset;
  vma->flags_ = flags;
  vma->node_.value_ = (size_t)address;
  bb_insert(&mspace->mmaps_, &vma->node_);
  *pVma = vma;
  return 0;
}


/* Return a virtual memory area that match the one describe eventually by
  splitting existing.
  If the range of address match several area, the function return the lowest.
  If one part of the range is non-allocated, the function return ENOENT.
  */
int Memory_split(memspace_t * mspace, void *address, size_t length, memvmap_t **pVma) {
  memvmap_t *cursor;
  memvmap_t *prev;
  if (address == NULL || length == 0 || length > MEM_VMA_LENGTH) {
    return EINVAL;
  }
  cursor = bb_search_less(&mspace->mmaps_, (size_t)address + length, memvmap_t, node_);
  while (cursor->node_.value_ > (size_t)address) {
    prev = bb_previous(&cursor->node_, memvmap_t, node_);
    // TODO Previous must be adjacent
  }

  return ENOMEM;
}
