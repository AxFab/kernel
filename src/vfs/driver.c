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
 *      File system driver NTFS.
 */
#include <smkos/kfs.h>


/* Map data from a block device. MAY BLOCK */
void *dev_map(device_t* dev, size_t lba, size_t cnt)
{
  char *buf; // = (char*)kalloc(dev->block_ * cnt);
  // dev_read_block(dev, buf, lba, cnt);
  return buf;
}

void dev_unmap(void* add)
{
  // kfree(add);
}

/* MAY BLOCK */
void dev_sync(device_t *dev)
{
}


