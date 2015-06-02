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
 *      Stub driver replace frame buffer by bitmap file.
 */
#include <smkos/kfs.h>
#include <smkos/compiler.h>
#include <stdio.h>
#include <stdlib.h>



PACK(struct BMP_Header {
  uint16_t sign_;
  uint32_t size_;
  uint32_t reserved_;
  uint32_t data_;
  uint32_t hdSize_;
  uint32_t width_;
  uint32_t height_;
  uint16_t planes_;
  uint16_t bitsPerPixel_;
  uint32_t compression_;
  uint32_t dataLength_;
  uint32_t horizResol_;
  uint32_t vertResol_;
  uint32_t colorCount_;
  uint32_t colorImpor_;
});

uint32_t *bmp_ptr = NULL;

// int BMP_map (kInode_t *ino, size_t offset, page_t *phys)
// {
//   return 0;
// }

int BMP_read (kInode_t *ino, void *bucket, size_t length, size_t lba)
{
  if (lba == 0)
    bmp_ptr = (uint32_t *)bucket;

  // memcpy (bucket, &((char*)ino->dev->data_)[lba], length);
  return 0;
}

int BMP_write (kInode_t *ino, void *bucket, size_t length, size_t lba)
{
  // memcpy (bucket, &((char*)ino->dev_->data_)[lba], length);
  return 0;
}


void BMP_sync (kInode_t *ino)
{
  struct BMP_Header head;
  FILE *fp;
  int i;

  if (!bmp_ptr)
    return;

  fp = fopen(SD_DIR "/vga.bmp", "w");

  memset (&head, 0, sizeof(head));
  head.sign_ = 0x4d42;
  head.size_ = 800 * 600 * 4 + sizeof(head);
  head.data_ = sizeof(head);
  head.hdSize_ = 40;
  head.width_ = 800;
  head.height_ = 600;
  head.planes_ = 1;
  head.bitsPerPixel_ = 32;
  head.bitsPerPixel_ = 32;
  head.dataLength_ = 800 * 600 * 4;
  head.horizResol_ = 96;
  head.vertResol_ = 96;

  fwrite (&head, 1, sizeof(head), fp);

  for (i = 599; i >= 0; --i)
    fwrite (&bmp_ptr[i * 800], 4, 800, fp);

  fclose(fp);
}


/* ----------------------------------------------------------------------- */
int BMP_mount (kInode_t *dev, const char *name)
{
  time_t now = time(NULL);
  SMK_stat_t stat;

  if (dev != NULL)
    return ENODEV;

  memset(&stat, 0, sizeof(stat));
  stat.major_ = BMP_No;
  stat.mode_ = S_IFBLK | 0755;
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.length_ = 800 * 600 * 4;
  stat.block_ = 800 * 4;

  create_device("Fb0", NULL, &stat, NULL);
  return 0;
}


/* ----------------------------------------------------------------------- */
void BMP(kDriver_t *driver)
{
  driver->major_ = BMP_No;
  driver->name_ = strdup("BMP");
  driver->mount = BMP_mount;
  driver->read = BMP_read;
}

