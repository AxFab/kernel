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
 *      File system driver ISO 9660.
 */
#include <smkos/kfs.h>


/* Identificators for volume descriptors */
#define ISO9660_STD_ID1    0x30444300
#define ISO9660_STD_ID2    0x00003130
#define ISO9660_VOLDESC_BOOT 0
#define ISO9660_VOLDESC_PRIM 1
#define ISO9660_VOLDESC_SUP  2
#define ISO9660_VOLDESC_VOL  3
#define ISO9660_VOLDESC_TERM 255


/** Structure of a Directory entry in Iso9660 FS */
PACK(struct ISO_dirEntry {
  uint8_t lengthRecord;
  char extLengthRecord;
  int locExtendLE;
  int locExtendBE;
  int dataLengthLE;
  int dataLengthBE;
  char recordYear;
  char recordMonth;
  char recordDay;
  char recordHour;
  char recordMinute;
  char recordSecond;
  char recordTimelag;
  char fileFlag;
  char fileUnitSize;
  char gapSize;
  short volSeqNumLE;
  short volSeqNumBE;
  char lengthFileId;
  char fileId[1];
});


/** Structure of the Primary volume descriptor in Iso9660 FS */
PACK(struct ISO_descFirst {
  char sysname[32];
  char volname[32];
  char unused1[8];
  int volSpaceSizeLE;
  int volSpaceSizeBE;
  char unused2[32];
  short volSetSizeLE;
  short volSetSizeBE;
  short volSeqNumberLE;
  short volSeqNumberBE;
  short logicBlockSizeLE;
  short logicBlockSizeBE;
  int pathTableSizeLE;
  int pathTableSizeBE;
  int locOccLpathTable;
  int locOccOptLpathTable;
  int locOccMpathTable;
  int locOccOptMpathTable;
  struct ISO_dirEntry rootDir;
  char volsetname[128];
  char publishId[128];
  char dataPrepId[128];
  char applicationId[128];
  char otherId[128];
  char dateYears[4];
  char dateMonths[2];
  char dateDays[2];
  char dateHours[2];
  char dateMinutes[2];
  char dateSeconds[2];
  char dateHundredthsSec[2];
  char timelag;
});


struct ISO_info {
  time_t      created;
  char        bootable;
  char       *name;
  int         lbaroot;
  int         lgthroot;
  int         sectorCount;
  int         sectorSize;
};


/* ----------------------------------------------------------------------- */
int ISO_mount (kInode_t *dev, const char *name)
{
  int i;
  int inDesc = 1;
  int sec = 16;
  char cdName[128];
  uint8_t *address;
  uint32_t *addressInt;
  struct ISO_descFirst *firstDesc;
  struct ISO_info *volume = NULL;
  SMK_stat_t stat;
  kMemArea_t *area;
  time_t now = time(NULL);

  if (dev == NULL || !S_ISBLK(dev->stat_.mode_) || dev->stat_.block_ != 2048)
    return ENODEV;

  while (inDesc) {

    area = area_map_ino(kSYS.mspace_, dev, sec * 2048, 2048, 0);
    address = (uint8_t *)area->address_ + (sec * 2048 - area->offset_);
    addressInt = (uint32_t *)address;
    // kdump (address, 64);

    if ((addressInt[0] & 0xFFFFFF00) != ISO9660_STD_ID1 ||
        (addressInt[1] & 0x0000FFFF) != ISO9660_STD_ID2 ||
        (address[0] != ISO9660_VOLDESC_PRIM && !volume)) {
      if (volume)
        kfree (volume);

      area_unmap(kSYS.mspace_, area);
      return EBADF;
    }

    switch (address[0]) {
    case ISO9660_VOLDESC_PRIM:
      firstDesc =  (struct ISO_descFirst *)&address[8];
      volume = KALLOC(struct ISO_info);
      volume->bootable = 0;
      volume->created = 0;
      volume->sectorSize = firstDesc->logicBlockSizeLE;
      volume->sectorCount = firstDesc->volSpaceSizeLE;
      volume->lbaroot = firstDesc->rootDir.locExtendLE;
      volume->lgthroot = firstDesc->rootDir.dataLengthLE;
      // kprintf ("ROOT { %x - %x }\n", firstDesc->rootDir.locExtendLE, firstDesc->rootDir.dataLengthLE);
      // kprintf ("VOLUME NAME '%s'\n", firstDesc->volname);

      for (i = 127; i > 0; --i) {
        if (firstDesc->applicationId [i] == ' ')
          firstDesc->applicationId [i] = '\0';
        else
          break;
      }

      strcpy (cdName, firstDesc->applicationId);

      // kprintf ("iso9660] This disc is named '%s' \n", firstDesc->applicationId);

      //err = Fsys_ChangeVolumeName (prim->applicationId);
      break;

    case ISO9660_VOLDESC_BOOT:
      // kprintf ("iso9660] Bootable descriptor\n");
      volume->bootable = !0;
      break;

    case ISO9660_VOLDESC_TERM:
      // kprintf ("iso9660] Terminal descriptor\n");
      inDesc = 0;
      break;

    default:

      // kprintf ("iso9660] Bad volume descriptor id %d\n", buf[0]);
      if (volume)
        kfree (volume);

      area_unmap(kSYS.mspace_, area);
      return ENOSYS;
    }

    ++sec;
    area_unmap(kSYS.mspace_, area);
  }

  memset(&stat, 0, sizeof(stat));
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.major_ = ISO_No;
  stat.block_ = volume->sectorSize;
  stat.mode_ = S_IFDIR | 0555;
  stat.lba_ = volume->lbaroot;
  stat.length_ = volume->lgthroot;

  if (!name)
    name = cdName;

  kprintf (" - ISO] Mount device %s at %s.\n", dev->name_, name);
  create_device(name, dev, &stat, volume);
  return 0;
}


/* ----------------------------------------------------------------------- */
// int ISO_readdir (const _pFileNode fn) {

// _Error err;
// _pByte buff = (_pByte)kalloc (2048, 0);
// _Iso9660_FileInfo nInfo;
// _pIso9660_FileInfo pInfo = (_pIso9660_FileInfo)fn;
// _pIso9660_Directory dir = (_pIso9660_Directory)buff;
// nInfo.structSize = sizeof (_Iso9660_FileInfo);
// err = STO_readSectors (buff, pInfo->lba, 0, 1); //FS_Infos->lbaroot
// if (err) {
//   kfree (buff);
//   return err;
// }

// while (dir->lengthRecord) {
//   // The first 2 are usaly '.' and '..' hard link
//   if (dir->locExtendLE == pInfo->lba || dir->locExtendLE == pInfo->parentLba) {
//     dir = (_pIso9660_Directory)&(((_pByte)dir)[dir->lengthRecord]);
//     continue;
//   }
//   nInfo.attribute = (dir->fileFlag & 0x2 ? _FsysNode_Directory_ : 0);
//   nInfo.rights = _FsysRight_All_;
//   nInfo.length = dir->dataLengthLE;
//   nInfo.lba = dir->locExtendLE;
//   nInfo.parentLba = pInfo->lba;
//   err = Fsys_RegisterFile (dir->fileId, (_pFileNode)&nInfo);
//   if (err) {
//     kfree (buff);
//     return err;
//   }
//   dir = (_pIso9660_Directory)&(((_pByte)dir)[dir->lengthRecord]);
// }
// kfree (buff);
// return __Err_None_;
// }


/* ----------------------------------------------------------------------- */
int ISO_read(kInode_t *fp, void *buffer, size_t length, size_t offset)
{
  return fs_block_read(fp->dev_->underlyingDev_, buffer, length, fp->stat_.lba_ * 2048 + offset);
}


/* ----------------------------------------------------------------------- */
int ISO_lookup (const char *name, kInode_t *dir, SMK_stat_t *stat)
{
  int i;
  char *filename = kalloc(PATH_MAX);
  size_t sec = dir->stat_.lba_;
  struct ISO_dirEntry *entry;
  // struct ISO_info* volume = (struct ISO_info*)dir->dev_->data_;
  uint8_t *address;
  kMemArea_t *area;

  // kprintf ("iso9660] Search %s on dir at lba[%x]\n", name, sec);
  // kprintf ("iso9660] Read sector %d on %s \n", sec, dir->name_);

  area = area_map_ino(kSYS.mspace_, dir->dev_->underlyingDev_->dev_->ino_, sec * 2048, 2048, 0);
  address = (uint8_t *)area->address_ + (sec * 2048 - area->offset_);

  //if (KLOG_FS) kprintf ("iso9660] Done\n");

  // Skip the first two entries '.' and '..'
  entry = (struct ISO_dirEntry *)address;
  entry = (struct ISO_dirEntry *) & (((char *)entry)[entry->lengthRecord]);
  entry = (struct ISO_dirEntry *) & (((char *)entry)[entry->lengthRecord]);

  while (entry->lengthRecord) {
    memcpy (filename, entry->fileId, entry->lengthFileId);
    filename[(int)entry->lengthFileId] = '\0';

    if (filename[entry->lengthFileId - 2 ] == ';')
      filename[entry->lengthFileId - 2] = '\0';

    // kprintf ("iso9660] See entry: '%s' \n", filename);

    if (strcmpi(name, filename) == 0) {
      // stat->atime_ =
      memset(stat, 0, sizeof (*stat));
      stat->major_ = dir->stat_.major_;
      stat->minor_ = dir->stat_.minor_;
      stat->block_ = dir->stat_.block_;
      stat->mode_ = (entry->fileFlag & 2 ? S_IFDIR : S_IFREG ) | 0555;
      stat->lba_ = entry->locExtendLE;
      stat->length_ = entry->dataLengthLE;
      area_unmap(kSYS.mspace_, area);
      kfree(filename);
      return 0;
    }

    entry = (struct ISO_dirEntry *) & (((char *)entry)[entry->lengthRecord]);
  }

  area_unmap(kSYS.mspace_, area);
  kfree(filename);
  return ENOENT;
}

/* ----------------------------------------------------------------------- */
void ISO9660(kDriver_t *driver)
{
  driver->major_ = ISO_No;
  driver->name_ = strdup("iso9660");
  driver->mount = ISO_mount;
  driver->lookup = ISO_lookup;
  driver->read = ISO_read;
  // driver->readdir = ISO_readdir;
}


