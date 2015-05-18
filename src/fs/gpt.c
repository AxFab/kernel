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
 *      Driver for MBR and GPT.
 */
#include <smkos/kfs.h>

PACK(struct GPT_mbrEntry {
  char status_;
  char firstCHS_[3];
  char partitionType_;
  char lastCHS_[3];
  uint32_t firstLBA_;
  uint32_t sectors_;
});

int GPT_count = 0;

/* ----------------------------------------------------------------------- */
int GPT_mount (kInode_t *dev, const char *name)
{
  int idx;
  char subname [10];
  SMK_stat_t stat;
  struct GPT_mbrEntry *entry;
  unsigned char *address;
  kMemArea_t *area;
  time_t now = time(NULL);

  if (dev == NULL || !S_ISBLK(dev->stat_.mode_) || dev->stat_.block_ != 512)
    return ENODEV;

  area = area_map_ino(kSYS.mspace_, dev, 0, 512, 0);
  area->at_ = __AT__;
  address = (unsigned char *)area->address_;

  if (address[510] != 0x55 || address[511] != 0xAA) {
    area_unmap(kSYS.mspace_, area);
    return ENOSYS;
  }

  memset(&stat, 0, sizeof(stat));
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.block_ = dev->stat_.block_;

  entry = (struct GPT_mbrEntry *)&address[446];

  for (idx = 0; idx < 4; ++idx, ++entry) {
    if (entry->sectors_ == 0)
      continue;

    stat.lba_ = entry->firstLBA_;
    stat.length_ = entry->sectors_;
    stat.major_ = GPT_No;
    stat.minor_ = ++GPT_count;
    stat.mode_ = S_IFBLK | 0700;
    snprintf(subname, 10, "%s%d", dev->name_, idx);
    create_device(subname, dev, &stat, NULL);
  }

  area_unmap(kSYS.mspace_, area);
  return 0;
}


/* ----------------------------------------------------------------------- */
int GPT_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  return fs_block_read(fp->dev_->underlyingDev_, buffer, length, fp->stat_.lba_ * fp->stat_.block_ + offset);
}


/* ----------------------------------------------------------------------- */
void GPT(kDriver_t *driver)
{
  driver->major_ = GPT_No;
  driver->name_ = strdup("gpt/mbr");
  driver->mount = GPT_mount;
  driver->read = GPT_read;
  //driver->map = GPT_map;
  //driver->sync = GPT_sync;
}


