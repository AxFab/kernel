#include "iso.h"

#include <stdio.h>
#include <fcntl.h>
// isoVolumeInfo_t* FS_Infos = 0;

// kVolume_t isoFs = {
//   "iso9660",
//   &isoMount, NULL, NULL,
//   &isoLookup, &isoRead, NULL, NULL,
//   NULL, NULL, NULL, NULL, NULL,
// };


// int iso_Mount (kFsys_t* fs, kStat_t* file)
// {
//   if (!fs->dev_)
//     return ENOTBLK;

//   if (fs->dev_->block_ != 2048)
//     return EBADF;


//   fs->block_ = 2048;
//   fs->cluster_ = 2048;

//   fs->lookup = isoLookup;
//   fs->read = isoRead;

//   strncpy (fs->name_, "Iso 9660 Fs", 16);

//   kStat_t root;
//   return isoMount (fs->dev_, &root);
// }


isoVolume_t* volInfo;

int isoMount (kFsys_t* fs, kStat_t* root)
{
  int i;
  int inDesc = 1;
  int sec = 16;
  uint8_t* buf;
  uint32_t* bufi;
  isoFstDescriptor_t* firstDesc;
  kDevice_t* dev = fs->dev_;

  if (!dev)
    return ENOTBLK;

  if (dev->block_ != 2048)
    return EBADF;

  fs->block_ = 2048;
  fs->cluster_ = 2048;
  fs->lookup = isoLookup;
  fs->read = isoRead;
  strncpy (fs->name_, "Iso 9660 Fs", 16);
  buf = malloc (2048);
  bufi = (uint32_t*)buf;
  memset (buf, 0, 2048);

  while (inDesc) {
    // lseek (dev, sec * 2048, SEEK_SET);
    // read (2048, buf, dev);
    // kprintf ("iso9660] Read sector %d \n", sec);
    kFs_ReadBlock (dev, buf, sec * 2048, 2048);
    // FIXME Read block don't use caching pages

    if ((bufi[0] & 0xFFFFFF00) != ISO9660_STD_ID1 ||
        (bufi[1] & 0x0000FFFF) != ISO9660_STD_ID2 ||
        (buf[0] != ISO9660_VOLDESC_PRIM && !volInfo)) {
      free (buf);
      return EBADF;
    }

    switch (buf[0]) {
      case ISO9660_VOLDESC_PRIM:
        firstDesc =  (isoFstDescriptor_t*)&buf[8];
        volInfo = (isoVolume_t*)malloc (sizeof(isoVolume_t));
        volInfo->bootable = 0;
        volInfo->created = 0;
        volInfo->sectorSize = firstDesc->logicBlockSizeLE;
        volInfo->sectorCount = firstDesc->volSpaceSizeLE;
        volInfo->lbaroot = firstDesc->rootDir.locExtendLE;
        volInfo->lgthroot = firstDesc->rootDir.dataLengthLE;
        // kprintf ("ROOT { %x - %x }\n", firstDesc->rootDir.locExtendLE, firstDesc->rootDir.dataLengthLE);
        // kprintf ("VOLUME NAME '%s'\n", firstDesc->volname);

        for (i = 127; i > 0; --i)
          if (firstDesc->applicationId [i] == ' ')
            firstDesc->applicationId [i] = '\0';

          else
            break;

        kprintf ("iso9660] This disc is named '%s' \n", firstDesc->applicationId);
        //err = Fsys_ChangeVolumeName (prim->applicationId);
        break;
      case ISO9660_VOLDESC_BOOT:
        // kprintf ("iso9660] Bootable descriptor\n");
        volInfo->bootable = !0;
        break;
      case ISO9660_VOLDESC_TERM:
        // kprintf ("iso9660] Terminal descriptor\n");
        inDesc = 0;
        break;
      default:
        // kprintf ("iso9660] Bad volume descriptor id %d\n", buf[0]);
        free(buf);
        return __seterrno(ENOSYS);
    }

    ++sec;
  }

  root->dev_ = NULL;
  root->mode_ = S_IFDIR | 0555;
  root->lba_ = volInfo->lbaroot;
  root->length_ = volInfo->lgthroot;
  // nInfo.parentLba = 0;
  free (buf);
  return __noerror ();
}


int isoLookup(const char* name, kInode_t* dir, kStat_t* file)
{
  char filename[PATH_MAX];
  size_t sec = dir->stat_.lba_;
  isoDirEntry_t* entry;
  char* buf = malloc (2048);
  // entry
  // kprintf ("iso9660] Search %s on dir at lba[%x]\n", name, dir->lba_);
  // kprintf ("iso9660] Read sector %d \n", sec);
  kFs_ReadBlock (dir->fs_->dev_, buf, sec * 2048, 2048);
  entry = (isoDirEntry_t*)buf;
  // Skip the first two entries
  entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);
  entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);

  while (entry->lengthRecord) {
    memcpy (filename, entry->fileId, entry->lengthFileId);
    filename[(int)entry->lengthFileId] = '\0';

    if (filename[entry->lengthFileId - 2 ] == ';')
      filename[entry->lengthFileId - 2] = '\0';

    // kprintf ("iso9660] See entry: '%s' \n", filename);
    if (strcmp(name, filename) == 0) {
      file->dev_ = NULL;

      if (entry->fileFlag & 2)
        file->mode_ = S_IFDIR | 0555;

      else
        file->mode_ = S_IFREG | 0555;

      file->lba_ = entry->locExtendLE;
      file->length_ = entry->dataLengthLE;
      free (buf);
      return __noerror();
    }

    entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);
  }

  free (buf);
  return ENOENT;
}

int isoRead (kInode_t* fp, void* buffer, off_t offset, size_t length)
{
  size_t sec = fp->stat_.lba_;
  kFs_ReadBlock (fp->fs_->dev_, buffer, sec * 2048 + offset, length);
  // TODO Respect the limit of the file !!!
  // readBlk (buffer, length, sec * 2048 + offset, fp->dev_);
  // kdump (buffer, 10);
  return 0;
}


// _Error iso9660_readdir (const _pFileNode fn) {

//   _Error err;
//   _pByte buff = (_pByte)malloc (2048);
//   _Iso9660_FileInfo nInfo;
//   _pIso9660_FileInfo pInfo = (_pIso9660_FileInfo)fn;
//   _pIso9660_Directory dir = (_pIso9660_Directory)buff;
//   nInfo.structSize = sizeof (_Iso9660_FileInfo);
//   err = STO_readSectors (buff, pInfo->lba, 0, 1); //FS_Infos->lbaroot
//   if (err) {
//     free (buff);
//     return err;
//   }

//   while (dir->lengthRecord) {
//     // The first 2 are usaly '.' and '..' hard link
//     if (dir->locExtendLE == pInfo->lba || dir->locExtendLE == pInfo->parentLba) {
//       dir = (_pIso9660_Directory)&(((_pByte)dir)[dir->lengthRecord]);
//       continue;
//     }
//     nInfo.attribute = (dir->fileFlag & 0x2 ? _FsysNode_Directory_ : 0);
//     nInfo.rights = _FsysRight_All_;
//     nInfo.length = dir->dataLengthLE;
//     nInfo.lba = dir->locExtendLE;
//     nInfo.parentLba = pInfo->lba;
//     err = Fsys_RegisterFile (dir->fileId, (_pFileNode)&nInfo);
//     if (err) {
//       free (buff);
//       return err;
//     }
//     dir = (_pIso9660_Directory)&(((_pByte)dir)[dir->lengthRecord]);
//   }
//   free (buff);
//   return __Err_None_;
// }

// _Error iso9660_readfile (const _pFileNode fn, FILE* fp, int offset, int count) {

//   _Error err;
//   _pByte buff = (_pByte)malloc (ALIGN(count, 2048));
//   _pIso9660_FileInfo pInfo = (_pIso9660_FileInfo)fn;
//   if (offset + count > pInfo->length)
//     return __FsErr_Outbound_;

//   err = STO_readSectors (buff, pInfo->lba, 0, ALIGN(count, 2048) / 2048); //FS_Infos->lbaroot
//   if (err) {
//     free (buff);
//     return err;
//   }
//   fwrite (buff, count, 1, fp);

//   free (buff);
//   return __Err_None_;
// }

