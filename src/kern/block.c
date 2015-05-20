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
 *      Device support and driver managment.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/drivers.h>

/* ----------------------------------------------------------------------- */
int fs_block_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  int err;
  /// @todo should be asynchrone
  klock (&fp->lock_);
  err = open_fs (fp);

  if (err)
    return err;

  err = fp->dev_->fs_->read(fp, buffer, length, offset);
  close_fs (fp);
  return err;
}


/* ----------------------------------------------------------------------- */
ssize_t fs_reg_read(kInode_t *ino, void *buf, size_t lg, ssize_t offset)
{
  size_t bytes = 0;
  size_t cap = 0;
  void *address;
  kMemArea_t *area;

  assert (S_ISREG(ino->stat_.mode_) || S_ISBLK(ino->stat_.mode_));

  while (lg > 0) {

    if ((size_t)offset >= ino->stat_.length_)
      break;

    /* Get Address */
    area = area_map_ino(kSYS.mspace_, ino, ALIGN_DW(offset, PAGE_SIZE), PAGE_SIZE, 0);
    area->at_ = __AT__;
    address = (void *)(area->address_ + (offset - area->offset_));

    /* Capacity ahead */
    cap = (area->limit_ - area->address_) + (offset - area->offset_);
    cap = MIN(cap, ino->stat_.length_ - offset);
    cap = MIN(cap, lg);

    if (cap == 0) {
      area_unmap(kSYS.mspace_, area);
      break;
    }

    /* Copy data */
    memcpy (buf, address, cap);
    lg -= cap;
    offset += cap;
    bytes += cap;
    buf = ((char *)buf) + cap;
  }

  area_unmap(kSYS.mspace_, area);
  return bytes;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
