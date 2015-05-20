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
 *      Driver for RAID0.
 */
#include <smkos/kfs.h>


/* ----------------------------------------------------------------------- */
int RAID0_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  int i, err;
  size_t j;
  kInode_t **inodes = NULL;

  for (i = 0, j = 0; j < length; ++i, j += 512)
    err = fs_block_read(inodes[i], &((char *)buffer)[j], 512, ALIGN_DW((offset + j) / 2, 512));

  return err;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
