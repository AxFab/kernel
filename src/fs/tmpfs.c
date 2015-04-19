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
#include <smkos/kernel.h>
#include <smkos/core.h>
#include <smkos/io.h>

/* ----------------------------------------------------------------------- */
int TMPFS_mount(kInode_t* dev, const char *name)
{
  if (dev != NULL)
    return ENOSYS;
  return EIO;
}

/* ----------------------------------------------------------------------- */
int TMPFS_create(const char *name, kInode_t *dir, int mode, size_t lg, SMK_stat_t *stat)
{
  time_t now = time(NULL);

  memset(stat, 0, sizeof(*stat));
  stat->atime_ = now;
  stat->ctime_ = now;
  stat->mtime_ = now;
  stat->block_ = PAGE_SIZE;
  stat->mode_ = mode;

  switch(mode & S_IFMT) {
    case S_IFREG:
    case S_IFDIR:
      return 0;

    default:
      return EINVAL;
  }
}


/* ----------------------------------------------------------------------- */
void TMPFS (kDriver_t* driver)
{
  driver->type_ = KDR_FS;
  driver->name_ = strdup("tmpfs");
  driver->mount = TMPFS_mount;
  driver->create = TMPFS_create;
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
