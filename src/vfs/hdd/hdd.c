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
 *      Driver for Um HDD images.
 */
#include <smkos/kfs.h>

#include <stdio.h>

FILE *fpa, *fpc;

/* ----------------------------------------------------------------------- */
int HDD_mount (kInode_t *dev, const char *name)
{
  time_t now = time(NULL);
  SMK_stat_t stat;

  if (dev)
    return ENOSYS;

  memset(&stat, 0, sizeof(stat));

  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.major_ = HDD_No;
  stat.mode_ =  S_IFBLK | 0770;

  fpa = fopen(SD_DIR "/hdd.img", "rb+");

  if (fpa) {
    fseek(fpa, 64 * _Mb_ - 1, SEEK_SET);
    fwrite("", 1, 1, fpa);
    fseek(fpa, 0, SEEK_END);
    fflush(fpa);
    stat.length_ = ftell(fpa);
    stat.block_ = 512;
    stat.minor_ = 0;
    create_device("sda", dev, &stat, fpa);
  }

  fpc = fopen(SD_DIR "/OsCore.iso", "rb");

  if (fpc) {
    fseek(fpc, 0, SEEK_END);
    stat.length_ = ftell(fpc);
    stat.block_ = 2048;
    stat.minor_ = 2;
    create_device("sdc", dev, &stat,  fpc);
  }

  return 0;
}

/* ----------------------------------------------------------------------- */
int HDD_unmount (kInode_t *dev, void *data)
{
  FILE *fio = (FILE *)data;
  fclose(fio);
  return 0;
}

/* ----------------------------------------------------------------------- */
int HDD_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  FILE *fio = (FILE *)fp->dev_->data_;
  fseek(fio, offset, SEEK_SET);
  fread(buffer, length, 1, fio);
  return 0;
}


/* ----------------------------------------------------------------------- */
int HDD_write(kInode_t *fp, const void *buffer, size_t length, size_t offset)
{
  FILE *fio = (FILE *)fp->dev_->data_;
  fseek(fio, offset, SEEK_SET);
  fwrite(buffer, length, 1, fio);
  return 0;
}


/* ----------------------------------------------------------------------- */
void HDD(kDriver_t *driver)
{
  driver->major_ = HDD_No;
  driver->name_ = strdup("hdd");
  driver->mount = HDD_mount;
  driver->unmount = HDD_unmount;
  driver->read = HDD_read;
  driver->write = HDD_write;
}

