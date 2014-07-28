#ifndef FS_ISO9660_ISO_H__
#define FS_ISO9660_ISO_H__

#include <kernel/inodes.h>

// Identificators for volume descriptors
#define ISO9660_STD_ID1    0x30444300
#define ISO9660_STD_ID2    0x00003130
#define ISO9660_VOLDESC_BOOT 0
#define ISO9660_VOLDESC_PRIM 1
#define ISO9660_VOLDESC_SUP  2
#define ISO9660_VOLDESC_VOL  3
#define ISO9660_VOLDESC_TERM 255

// ========================================================
// ISO9660 - Specification structures


typedef struct isoDirEntry          isoDirEntry_t;
typedef struct isoFstDescriptor     isoFstDescriptor_t;
typedef struct isoVolume            isoVolume_t;


// Structure of a Directory entry in Iso9660 FS
struct __attribute__ ((__packed__)) isoDirEntry {
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
};

// Structure of the Primary volume descriptor in Iso9660 FS
struct __attribute__ ((__packed__)) isoFstDescriptor {
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
  isoDirEntry_t rootDir;
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
};

// ========================================================
// ISO9660 - Intern structures

// Store info about the volume
struct isoVolume {
  time_t      created;
  char        bootable;
  char*       name;
  int         lbaroot;
  int         lgthroot;
  int         sectorCount;
  int         sectorSize;
  kInode_t*   dev;
};


int ISO_Mount (kInode_t* dev, kInode_t* mnt);
int ISO_Lookup(const char* name, kInode_t* dir, kStat_t* file);
int ISO_Read(kInode_t* fp, void* buffer, size_t count, size_t lba);
int ISO_Write();


#endif /* FS_ISO9660_ISO_H__ */
