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


struct NTFS_info {
  int clust_size_;
  int hidden_sec_;
  size_t sec_cnt_;
  size_t clust_for_mft_;
  size_t clust_for_mft_mirr_;
  int clust_per_file_recd_;
  int clust_per_idx_;
  unsigned long long serial_num_;
};

PACK (struct NTFS_bootsec {
  char jmp_[3];
  unsigned long long id_;
  unsigned short byte_per_sec_;
  unsigned char sec_per_clust_;
  unsigned short resv_sec_;
  char resvd1_[3];
  short unused1_;
  unsigned char media_;
  char resvd2_[2];
  unsigned short sec_pre_track_;
  unsigned short head_cnt_;
  unsigned int hidden_sec_;
  char unused2_[8];
  unsigned long long sec_cnt_;
  unsigned long long clust_for_mft_; // Logical Cluster Number for the file $MFT
  unsigned long long clust_for_mft_mirr_;
  unsigned int clust_per_file_recd_;
  unsigned char clust_per_idx_;
  char unused3_[3];
  unsigned long long serial_num_;
  unsigned int checksum_;
  char btcode_[426];
  unsigned short end_marker_;
});


int NTFS_mount(device_t *dev)
{
  struct NTFS_bootsec* bsec;
  if (dev == NULL)
    return -1;

  bsec = (struct NTFS_bootsec*)dev_map(dev, 0, 1);

  if (bsec->id_ != 0x202020205346544EULL) {
    dev_unmap(bsec);
    return -1;
  }

  // kdump(bsec, 32);
  return 0;
}


