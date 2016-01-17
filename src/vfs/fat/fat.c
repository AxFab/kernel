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
 *      File system driver FAT12, FAT16, FAT32 and exFAT.
 */
#include <smkos/kfs.h>


/* Identificators for volume descriptors */
#define FAT12 12
#define FAT16 16
#define FAT32 32

#define ATTR_READ_ONLY			0x01
#define ATTR_HIDDEN				0x02
#define ATTR_SYSTEM				0x04
#define ATTR_VOLUME_ID			0x08
#define ATTR_DIRECTORY			0x10
#define ATTR_ARCHIVE			0x20
#define ATTR_LONG_NAME			0x0F
#define ATTR_LONG_NAME_MASK		0x3F


#define CLUSTER_OF(v,s)   ((((s) - (v)->FirstDataSector) / (v)->SecPerClus) + 2)
#define SECTOR_OF(v,s)    ((((s) - 2) * (v)->SecPerClus) + (v)->FirstDataSector)
#define FSECTOR_FROM(v,s) ((((((((s) - (v)->FirstDataSector) / (v)->SecPerClus) + 2)) - 2) * (v)->SecPerClus) + (v)->FirstDataSector)

PACK(struct BPB_Struct {
	unsigned char	BS_jmpBoot [3];
	char			BS_OEMName [8];
	unsigned short  BPB_BytsPerSec;
	unsigned char	BPB_SecPerClus;
	unsigned short	BPB_ResvdSecCnt;
	unsigned char	BPB_NumFATs;
	unsigned short	BPB_RootEntCnt;
	unsigned short	BPB_TotSec16;
	unsigned char	BPB_Media;
	unsigned short	BPB_FATSz16;
	unsigned short	BPB_SecPerTrk;
	unsigned short	BPB_NumHeads;
	unsigned int	BPB_HiddSec;
	unsigned int	BPB_TotSec32;

	unsigned char	BS_DrvNum;
	unsigned char	BS_Reserved1;
	unsigned char	BS_BootSig;
	unsigned int	BS_VolID;
	char			BS_VolLab [11];
	char			BS_FilSysType [8];
});

PACK(struct BPB_Struct32 {
	unsigned char	BS_jmpBoot [3];
	char			BS_OEMName [8];
	unsigned short	BPB_BytsPerSec;
	unsigned char	BPB_SecPerClus;
	unsigned short	BPB_ResvdSecCnt;
	unsigned char	BPB_NumFATs;
	unsigned short	BPB_RootEntCnt;
	unsigned short	BPB_TotSec16;
	unsigned char	BPB_Media;
	unsigned short	BPB_FATSz16;
	unsigned short	BPB_SecPerTrk;
	unsigned short	BPB_NumHeads;
	unsigned int	BPB_HiddSec;
	unsigned int	BPB_TotSec32;

	unsigned int	BPB_FATSz32;
	unsigned short	BPB_ExtFlags;
	unsigned short	BPB_FSVer;
	unsigned int	BPB_RootClus;
	unsigned short	BPB_FSInfo;
	unsigned short	BPB_BkBootSec;
	char			BPB_Reserved [12];
	
	unsigned char	BS_DrvNum;
	unsigned char	BS_Reserved1;
	unsigned char	BS_BootSig;
	unsigned int	BS_VolID;
	char			BS_VolLab [11];
	char			BS_FilSysType [8];
});



PACK(struct FAT_ShortEntry {
	unsigned char DIR_Name[11];
	unsigned char	DIR_Attr;
	unsigned char	DIR_NTRes;
	unsigned char	DIR_CrtTimeTenth;
	unsigned short	DIR_CrtTime;
	unsigned short	DIR_CrtDate;
	unsigned short	DIR_LstAccDate;
	unsigned short	DIR_FstClusHi;
	unsigned short	DIR_WrtTime;
	unsigned short	DIR_WrtDate;
	unsigned short	DIR_FstClusLo;
	unsigned int	DIR_FileSize;
});

PACK(struct FAT_LongNameEntry {
	unsigned char	LDIR_Ord;
	unsigned short	LDIR_Name1[5];
	unsigned char	LDIR_Attr;
	unsigned char	LDIR_Type;
	unsigned char	LDIR_Cheksum;
	unsigned short	LDIR_Name2[6];
	unsigned short	LDIR_FstClusLO;
	unsigned short	LDIR_Name3[2];
});



/** Structure for a volume in FAT FS */
struct FAT_Volume {

	char		name[48];
	char		label[8];
	long long	totalSize;
	long long	usedSpace;
	long long	freeSpace;
	unsigned int RootDirSectors;
	unsigned int FATSz;
	unsigned int FirstDataSector;
	unsigned int TotSec;
	unsigned int DataSec;
	unsigned int CountofClusters;
	unsigned int ResvdSecCnt;
	unsigned int BytsPerSec;
	int FATType;
	unsigned int RootEntry;
	int SecPerClus;
};


/* ----------------------------------------------------------------------- */
static char* FAT_build_short_name(struct FAT_ShortEntry *entrySh)
{
	int i;
	char str [9] = { 0 };
	memcpy (str, entrySh->DIR_Name, 8);
	// for (i=1; i<9; ++i)
	//   str[i] = tolower(str[i]);
	for (i=7; i>0; --i) {
		if (str[i] > 0x20)
			break;
		str[i] = 0;
	}

	return strdup (str);
}

/* ----------------------------------------------------------------------- */
static char* FAT_build_long_name(struct FAT_LongNameEntry *entryLg)
{
  uint16_t unicode;
	char part [72];
	int i=0, j=0;

	for (j=0; j<5; ++j) {
		unicode = entryLg->LDIR_Name1[j];
		if (unicode < 0x80)
			part[i++] = unicode & 0x7F;
		else
			part[i++] = '-';
  }

	for (; j<11; ++j) {
		unicode = entryLg->LDIR_Name2[j - 5];
		if (unicode < 0x80)
			part[i++] = unicode & 0x7F;
		else
			part[i++] = '-';
  }

	for (; j<13; ++j) {
		unicode = entryLg->LDIR_Name3[j - 11];
		if (unicode < 0x80)
			part[i++] = unicode & 0x7F;
		else
			part[i++] = '-';
	}

	part[i] = 0;
	return strdup(part);
}

/* ----------------------------------------------------------------------- */
static char* FAT_name_concat(char* s1, char* s2)
{
  int lg = strlen(s1) + strlen(s2) + 1;
  char* part = (char*)kalloc(lg);
  strncpy(part, s1, lg);
  strncat(part, s2, lg);
  return part;
}

/* ----------------------------------------------------------------------- */
static time_t FAT_read_time(unsigned short date, unsigned short time)
{
  struct tm datetime;
  datetime.tm_mday = date & 0x1F;
  datetime.tm_mon = (date >> 5) & 0x0F;
	datetime.tm_year = (date >> 9) + 80; // since 1900
  datetime.tm_sec = (time & 0x1F) << 1;
	datetime.tm_min = (time >> 5) & 0x3F;
	datetime.tm_hour = time >> 11;
  return mktime(&datetime);
}

static struct FAT_LongNameEntry * FAT_prepare_entries (const char *name, int mode, SMK_stat_t *stat)
{
  struct FAT_LongNameEntry * entries;
  // struct FAT_ShortEntry * shEntry;
  const char *lname = name;
  int i, j, eIdx = 0;
  int *ptr;
  int nlg = strlen (name); // @todo UTF-16
  int enCount = nlg / 13;
  if (nlg % 13)
    enCount++;

  entries = (struct FAT_LongNameEntry *)kalloc ((enCount + 1) * sizeof(struct FAT_LongNameEntry));
  // shEntry = (struct FAT_ShortEntry *)(&entries[enCount]);

  for (i=0,j=0; i < nlg; ++i, ++j) {
    if (j >= 13)
      eIdx++;

    if (j < 5)
      entries[eIdx].LDIR_Name1[j] = *lname; // @todo get from UTF-16 string!
    else if (j < 11)
      entries[eIdx].LDIR_Name2[j - 5] = *lname; // @todo get from UTF-16 string!
    else
      entries[eIdx].LDIR_Name3[j - 11] = *lname; // @todo get from UTF-16 string!
    lname++;
  }

  for (eIdx=0; eIdx < enCount; ++eIdx) {
    ptr = (int*)(&entries[eIdx]);
    entries[eIdx].LDIR_Attr = ATTR_LONG_NAME;
    entries[eIdx].LDIR_Ord = eIdx + 1;
    entries[eIdx].LDIR_Cheksum = XOR_32_TO_8 (ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3] ^ ptr[4] ^ ptr[5] ^ ptr[6] ^ ptr[7]);
  }

  // shEntry->DIR_Name[0] =
  // shEntry->DIR_Attr =

  return entries;
}

/* ----------------------------------------------------------------------- */
int FAT_mount (kInode_t *dev, const char *name)
{
  time_t now = time(NULL);
  uint8_t *address;
  kMemArea_t* area;
  struct SMK_stat stat;
	struct BPB_Struct *bpb;
	struct BPB_Struct32 *bpb32;
	struct FAT_Volume *mount;

  if (dev == NULL || !S_ISBLK(dev->stat_.mode_) || dev->stat_.block_ != 512)
    return ENODEV;

  area = area_map_ino(kSYS.mspace_, dev, 0, 512, 0);
  area->at_ = __AT__;
  address = (uint8_t *)area->address_;

  if (address[510] != 0x55 || address[511] != 0xAA) {
    area_unmap(kSYS.mspace_, area);
    return EBADF;
	}
	
	bpb = (struct BPB_Struct *)address;
	bpb32 = (struct BPB_Struct32 *)address;

  if (bpb->BS_jmpBoot[0] != 0xE9 && !(bpb->BS_jmpBoot[0] == 0xEB && bpb->BS_jmpBoot[2] == 0x90)) {
    area_unmap(kSYS.mspace_, area);
    return EBADF;
	}

  if (!POW2(bpb->BPB_BytsPerSec) || !POW2(bpb->BPB_SecPerClus) || bpb->BPB_ResvdSecCnt == 0) {
    area_unmap(kSYS.mspace_, area);
    return EBADF;
	}

  mount = KALLOC(struct FAT_Volume);
	mount->RootDirSectors = ((bpb->BPB_RootEntCnt * 32) + (bpb->BPB_BytsPerSec - 1)) / bpb->BPB_BytsPerSec;
	mount->FATSz = (bpb->BPB_FATSz16 != 0 ? bpb->BPB_FATSz16 : bpb32->BPB_FATSz32);
	mount->FirstDataSector = bpb->BPB_ResvdSecCnt + (bpb->BPB_NumFATs * mount->FATSz) + mount->RootDirSectors;
	mount->TotSec =  (bpb->BPB_TotSec16 != 0 ? bpb->BPB_TotSec16 : bpb->BPB_TotSec32);
	mount->DataSec = mount->TotSec - (bpb->BPB_ResvdSecCnt + (bpb->BPB_NumFATs * mount->FATSz) + mount->RootDirSectors);
  if (mount->FATSz == 0 || bpb->BPB_SecPerClus == 0 || mount->TotSec <= mount->DataSec) {
    kfree(mount);
    area_unmap(kSYS.mspace_, area);
    return EBADF;
  }

	mount->CountofClusters = mount->DataSec / bpb->BPB_SecPerClus;
	mount->FATType = (mount->CountofClusters < 4085 ? FAT12 : (mount->CountofClusters < 65525 ? FAT16 : FAT32));
	if (mount->FATType == FAT16) {
		mount->RootEntry = bpb->BPB_ResvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz16);
	}
	else {
		mount->RootEntry = ((bpb32->BPB_RootClus - 2) * bpb->BPB_SecPerClus) + mount->FirstDataSector;
	}
	mount->SecPerClus = bpb->BPB_SecPerClus;
	mount->ResvdSecCnt = bpb->BPB_ResvdSecCnt;
	mount->BytsPerSec = bpb->BPB_BytsPerSec;

	memcpy (mount->name, "UNNAMED", 48); // ERR
	mount->totalSize = (long long)mount->DataSec * 512;
	mount->usedSpace = 0;
	mount->freeSpace = 0;

	// kprintf ("DEBUG: FAT - byte per sector: %d\n", bpb->BPB_BytsPerSec);
  memset(&stat, 0, sizeof(stat));
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.major_ = FAT_No;
  stat.block_ = mount->SecPerClus * mount->BytsPerSec;
  stat.mode_ = S_IFDIR | 0555;
  stat.lba_ = mount->RootEntry;
  stat.length_ = 0;

  area_unmap(kSYS.mspace_, area);

  if (!name)
    name = mount->name;

  kprintf (" - FAT] Mount device %s at %s.\n", dev->name_, name);
  create_device(name, dev, &stat, mount);
  return 0;
}

/* ----------------------------------------------------------------------- */
int FAT_unmount (kInode_t *dev, void *info)
{
  kfree(info);
  return 0;
}


/* ----------------------------------------------------------------------- */
// int FAT_readdir (const _pFileNode fn) {
// }

/* ----------------------------------------------------------------------- */
int FAT_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  struct FAT_Volume* volume = (struct FAT_Volume*)fp->dev_->data_;
  size_t clustSz = volume->SecPerClus * volume->BytsPerSec;
  size_t lba = fp->stat_.lba_;
  uint8_t *fat;
  uint32_t fatEntry;
  size_t fileOff = 0;
  size_t fatCur = 0;
  kMemArea_t *fatArea = NULL;
  size_t cluster;
  size_t fatSec;
  size_t fatOff;

  for (;;) {
    if (fileOff + clustSz > offset && fileOff + clustSz < offset + length) {
      size_t cap = length;
      cap = MIN (cap, clustSz);
      if (offset > fileOff)
        cap = MIN (cap, clustSz - (offset - fileOff));
      if (fileOff + clustSz > fp->stat_.length_)
        cap = MIN (cap, fp->stat_.length_ - offset);
      fs_block_read(fp->dev_->underlyingDev_, buffer, cap, lba * 512 + offset);

      length -= cap;
      buffer = ((char*)buffer) + cap;
    }

    if (length <= 0) {
      if (fatArea)
        area_unmap(kSYS.mspace_, fatArea);
      return 0;
    }

    cluster = CLUSTER_OF(volume, lba);
    fatSec = volume->ResvdSecCnt + (cluster / 128);
    fatOff = cluster % 128;
    if (fatArea == NULL || fatSec != fatCur) {
      fatCur = fatSec;
      fatArea = area_map_ino(kSYS.mspace_, fp->dev_->underlyingDev_->dev_->ino_, fatSec * 512, 512, 0);
      fat = (uint8_t *)fatArea->address_;
    }

    fatEntry = ((uint32_t*)fat)[fatOff];
    if (fatEntry < 0xFFFFFFF) {
      fileOff += clustSz;
      lba = SECTOR_OF(volume, fatEntry);
      continue;
    }

    return EIO;
  }
}

/* ----------------------------------------------------------------------- */
int FAT_lookup (const char *name, kInode_t *dir, SMK_stat_t *stat)
{
  struct FAT_Volume* volume = (struct FAT_Volume*)dir->dev_->data_;
  size_t clustSz = volume->SecPerClus * volume->BytsPerSec;
  size_t lba = dir->stat_.lba_;
  uint8_t *fat;
  uint32_t fatEntry;
  size_t curCluster = 0;
  size_t cluster;
  size_t fatCur = 0;
  size_t fatSec;
  size_t fatOff;
  int idx;
  char* filename = NULL;
  struct FAT_ShortEntry *entrySh;
  kMemArea_t *fatArea = NULL;
  kMemArea_t *dirArea = NULL;

  for (;;) {
    cluster = CLUSTER_OF(volume, lba);
    if (dirArea == NULL || curCluster != cluster) {
      if (dirArea != NULL)
        area_unmap(kSYS.mspace_, dirArea);
      dirArea = area_map_ino(kSYS.mspace_, dir->dev_->underlyingDev_->dev_->ino_, SECTOR_OF(volume, cluster) * 512, clustSz, 0);
      fatSec = volume->ResvdSecCnt + (cluster / 128);
      fatOff = cluster % 128;
    }

    entrySh = (struct FAT_ShortEntry *)dirArea->address_;
    for (idx = 0; idx < 16 * volume->SecPerClus; entrySh++, idx++) {
      if (entrySh->DIR_Name[0] == 0xE5 || entrySh->DIR_Name[0] == 0x05 || entrySh->DIR_Attr == ATTR_VOLUME_ID)
        continue;

      if (entrySh->DIR_Name[0] == 0) {
        area_unmap(kSYS.mspace_, dirArea);
        if (fatArea)
          area_unmap(kSYS.mspace_, fatArea);
        if (filename)
          kfree(filename);
        return ENOENT;
      }

      if ((entrySh->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
        if (filename) {
          char* part = FAT_build_long_name((struct FAT_LongNameEntry *)entrySh);
          char* tmp = FAT_name_concat(part, filename);
          kfree(part);
          kfree(filename);
          filename = tmp;
        } else {
          filename = FAT_build_long_name((struct FAT_LongNameEntry *)entrySh);
        }
        continue;
      }

      if (!filename)
        filename = FAT_build_short_name(entrySh);

      if (strcmpi(name, filename) != 0) {
        kfree(filename);
        filename = NULL;
        continue;
      }

      stat->ctime_ = FAT_read_time(entrySh->DIR_CrtDate, entrySh->DIR_CrtTime);
      stat->atime_ = FAT_read_time(entrySh->DIR_LstAccDate, 0);
      stat->mtime_ = FAT_read_time(entrySh->DIR_WrtDate, entrySh->DIR_WrtTime);
      stat->mode_ = (entrySh->DIR_Attr & ATTR_DIRECTORY ? S_IFDIR: S_IFREG) | 0777;
      stat->lba_ = SECTOR_OF(volume, entrySh->DIR_FstClusLo | (entrySh->DIR_FstClusHi << 16));
      stat->length_ = entrySh->DIR_FileSize;

      // kprintf ("FAT] %s (%c - %s)\n", filename, (entrySh->DIR_Attr & ATTR_DIRECTORY ? 'd': '-'), kpsize(stat->length_));
      area_unmap(kSYS.mspace_, dirArea);
      if (fatArea)
        area_unmap(kSYS.mspace_, fatArea);
      kfree(filename);
      return 0;
    }

    if (fatArea == NULL || fatSec != fatCur) {
      fatCur = fatSec;
      fatArea = area_map_ino(kSYS.mspace_, dir->dev_->underlyingDev_->dev_->ino_, fatSec * 512, 512, 0);
      fat = (uint8_t *)fatArea->address_;
    }

    fatEntry = ((uint32_t*)fat)[fatOff];
    if (fatEntry < 0xFFFFFFF) {
      lba = SECTOR_OF(volume, fatEntry);
      continue;
    }

    area_unmap(kSYS.mspace_, dirArea);
    if (fatArea)
      area_unmap(kSYS.mspace_, fatArea);
    if (filename)
      kfree(filename);
    return ENOENT;
  }
}

/* ----------------------------------------------------------------------- */
int FAT_create (const char *name, kInode_t *dir, int mode, size_t lg, SMK_stat_t *stat)
{
  struct FAT_Volume* volume = (struct FAT_Volume*)dir->dev_->data_;
  size_t clustSz = volume->SecPerClus * volume->BytsPerSec;
  size_t lba = dir->stat_.lba_;
  uint8_t *fat;
  uint32_t fatEntry;
  size_t curCluster = 0;
  size_t cluster;
  size_t fatCur = 0;
  size_t fatSec;
  size_t fatOff;
  int idx;
  char* filename = NULL;
  struct FAT_ShortEntry *entrySh;
  struct FAT_LongNameEntry *entryLg;
  kMemArea_t *fatArea = NULL;
  kMemArea_t *dirArea = NULL;

  // We need how many longentry !?
  entryLg = FAT_prepare_entries(name, mode, stat);
  __unused(entryLg);

  for (;;) {
    cluster = CLUSTER_OF(volume, lba);
    if (dirArea == NULL || curCluster != cluster) {
      if (dirArea != NULL)
        area_unmap(kSYS.mspace_, dirArea);
      dirArea = area_map_ino(kSYS.mspace_, dir->dev_->underlyingDev_->dev_->ino_, SECTOR_OF(volume, cluster) * 512, clustSz, 0);
      fatSec = volume->ResvdSecCnt + (cluster / 128);
      fatOff = cluster % 128;
    }

    entrySh = (struct FAT_ShortEntry *)dirArea->address_;
    for (idx = 0; idx < 16 * volume->SecPerClus; entrySh++, idx++) {
      if (entrySh->DIR_Name[0] == 0xE5 || entrySh->DIR_Name[0] == 0x05 || entrySh->DIR_Attr == ATTR_VOLUME_ID)
        continue;

      if (entrySh->DIR_Name[0] == 0) {
        area_unmap(kSYS.mspace_, dirArea);
        if (fatArea)
          area_unmap(kSYS.mspace_, fatArea);
        if (filename)
          kfree(filename);
        return ENOENT;
      }

      if ((entrySh->DIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
        if (filename) {
          char* part = FAT_build_long_name((struct FAT_LongNameEntry *)entrySh);
          char* tmp = FAT_name_concat(part, filename);
          kfree(part);
          kfree(filename);
          filename = tmp;
        } else {
          filename = FAT_build_long_name((struct FAT_LongNameEntry *)entrySh);
        }
        continue;
      }

      if (!filename)
        filename = FAT_build_short_name(entrySh);

      if (strcmpi(name, filename) != 0) {
        kfree(filename);
        filename = NULL;
        continue;
      }

      stat->ctime_ = FAT_read_time(entrySh->DIR_CrtDate, entrySh->DIR_CrtTime);
      stat->atime_ = FAT_read_time(entrySh->DIR_LstAccDate, 0);
      stat->mtime_ = FAT_read_time(entrySh->DIR_WrtDate, entrySh->DIR_WrtTime);
      stat->mode_ = (entrySh->DIR_Attr & ATTR_DIRECTORY ? S_IFDIR: S_IFREG) | 0777;
      stat->lba_ = SECTOR_OF(volume, entrySh->DIR_FstClusLo | (entrySh->DIR_FstClusHi << 16));
      stat->length_ = entrySh->DIR_FileSize;

      // kprintf ("FAT] %s (%c - %s)\n", filename, (entrySh->DIR_Attr & ATTR_DIRECTORY ? 'd': '-'), kpsize(stat->length_));
      area_unmap(kSYS.mspace_, dirArea);
      if (fatArea)
        area_unmap(kSYS.mspace_, fatArea);
      kfree(filename);
      return 0;
    }

    if (fatArea == NULL || fatSec != fatCur) {
      fatCur = fatSec;
      fatArea = area_map_ino(kSYS.mspace_, dir->dev_->underlyingDev_->dev_->ino_, fatSec * 512, 512, 0);
      fat = (uint8_t *)fatArea->address_;
    }

    fatEntry = ((uint32_t*)fat)[fatOff];
    if (fatEntry < 0xFFFFFFF) {
      lba = SECTOR_OF(volume, fatEntry);
      continue;
    }

    area_unmap(kSYS.mspace_, dirArea);
    if (fatArea)
      area_unmap(kSYS.mspace_, fatArea);
    if (filename)
      kfree(filename);
    return ENOENT;
  }

}

/* ----------------------------------------------------------------------- */
void FATFS(kDriver_t *driver)
{
  driver->major_ = FAT_No;
  driver->name_ = strdup("FAT");
  driver->mount = FAT_mount;
  driver->unmount = FAT_unmount;
  driver->lookup = FAT_lookup;
  driver->read = FAT_read;
  driver->create = FAT_create;
  // driver->readdir = FAT_readdir;
}


