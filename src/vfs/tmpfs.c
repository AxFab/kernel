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
 *      File system driver tmpfs.
 */
#include <smkos/vfs.h>
#include <skc/stat.h>

int tmpfs_auto = 0;

int TMPFS_lookup(const TCHAR *name, inode_t* dir, inode_t *ino)
{
  return ENOENT;
}

int TMPFS_mknod(const TCHAR *name, inode_t* dir, inode_t *ino)
{  
  if (!S_ISREG(ino->mode_) && !S_ISDIR(ino->mode_) && S_ISTTY(ino->mode_))
    return -1;
  ino->no_ = ++tmpfs_auto;
  ino->major_ = 12;
  ino->minor_ = 0;
  ino->length_ = 0;
  ino->lba_ = 0;
  ino->block_ = 1;
  return 0;
}

void TMPFS_init(driver_t *drv)
{
  memset(drv, 0, sizeof(*drv));
  strcpy(drv->name_, "TMPFS");
  drv->major_ = 12;
  drv->compare = tcscmp;
  drv->mknod = TMPFS_mknod;
  drv->lookup = TMPFS_lookup;
}



