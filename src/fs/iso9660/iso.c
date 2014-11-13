#include "iso.h"

#include <stdio.h>
#include <fcntl.h>



kDevice_t isoOps = {
  {0}, 
  ISO_Lookup, ISO_Read, NULL, NULL,
  NULL, ISO_Write,
  NULL
};


int ISO_mount (kInode_t* dev, kInode_t* mnt, const char* mpoint)
{
  int i;
  int inDesc = 1;
  int sec = 16;
  char name[128];
  uint8_t* buf;
  uint32_t* bufi;
  isoFstDescriptor_t* firstDesc;
  kStat_t root = { 0 };
  root.mode_ = S_IFDIR | 0555;
  isoVolume_t* volInfo;

  if (dev->stat_.block_ != 2048)
    return EBADF;


  buf = (uint8_t*) malloc (2048);
  bufi = (uint32_t*)buf;
  while (inDesc) {
    feed_inode (dev, buf, 1, sec);

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
        volInfo->dev = dev;
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

        strcpy (name, firstDesc->applicationId);
        if (KLOG_FS) kprintf ("iso9660] This disc is named '%s' \n", firstDesc->applicationId);
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
        free (buf);
        return __seterrno(ENOSYS);
    }

     ++sec;
  }

  root.mode_ = S_IFDIR | 0555;
  root.lba_ = volInfo->lbaroot;
  root.length_ = volInfo->lgthroot;
  // FIXME Fill creation date
  if (mpoint)
    kfs_new_device (mpoint, mnt, &isoOps, (void*)volInfo, &root);
  else
    kfs_new_device (name, mnt, &isoOps, (void*)volInfo, &root);

  free (buf);
  return __noerror ();
}

int ISO_Write()
{
  return EROFS;
}

int ISO_Lookup(const char* name, kInode_t* dir, kStat_t* file)
{
  char filename[PATH_MAX];
  size_t sec = dir->stat_.lba_;
  isoDirEntry_t* entry;
  char* buf = malloc (2048);
  isoVolume_t* volInfo = (isoVolume_t*)dir->devinfo_;

  if (KLOG_FS) kprintf ("iso9660] Search %s on dir at lba[%x]\n", name, sec);
  if (KLOG_FS) kprintf ("iso9660] Read sector %d on %s \n", sec, volInfo->dev->name_);

  klock(&volInfo->dev->lock_);
  feed_inode (volInfo->dev, buf, 1, sec);
  if (KLOG_FS) kprintf ("iso9660] Done\n");

  // Skip the first two entries
  entry = (isoDirEntry_t*)buf;
  entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);
  entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);

  while (entry->lengthRecord) {
    memcpy (filename, entry->fileId, entry->lengthFileId);
    filename[(int)entry->lengthFileId] = '\0';

    if (filename[entry->lengthFileId - 2 ] == ';')
      filename[entry->lengthFileId - 2] = '\0';

    if (KLOG_FS) kprintf ("iso9660] See entry: '%s' \n", filename);
    if (strcmp(name, filename) == 0) {
      file->dev_ = volInfo->dev->stat_.ino_;
      // file->dblock_ = volInfo->dev->stat_.cblock_;
      file->block_ = 2048;

      if (entry->fileFlag & 2)
        file->mode_ = S_IFDIR | 0555;

      else
        file->mode_ = S_IFREG | 0555;

      file->lba_ = entry->locExtendLE;
      file->length_ = entry->dataLengthLE;
      free (buf);
      kunlock(&volInfo->dev->lock_); 
      return 0;
    }

    entry = (isoDirEntry_t*) & (((char*)entry)[entry->lengthRecord]);
  }

  free (buf);
  // @todo Pre-lock underlaying devices... brainstorming needed.
  kunlock(&volInfo->dev->lock_); 
  return ENOENT;
}

int ISO_Read(kInode_t* fp, void* buffer, size_t count, size_t lba)
{
  isoVolume_t* volInfo = (isoVolume_t*)fp->devinfo_;
  size_t sec = fp->stat_.lba_;
  size_t lg = ALIGN_UP (fp->stat_.length_, fp->stat_.block_) / fp->stat_.block_;
  if (lg < lba + count )
    count = lg - lba;

  if (count <= 0) {
    if (KLOG_FS) kprintf ("iso9660] File %s have size %d (request until %d)\n", fp->name_, lg, lba + count );
    return ERANGE;
  }

  if (KLOG_FS) kprintf ("iso9660] File %s, read dev %s at %d \n", fp->name_, volInfo->dev->name_, sec + lba );

  klock(&volInfo->dev->lock_);
  feed_inode (volInfo->dev, buffer, count, sec + lba);
  kunlock(&volInfo->dev->lock_);
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

