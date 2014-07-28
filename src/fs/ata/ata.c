#include "ata.h"


// ===========================================================================
kAtaDrive_t sdx[4];

kFileOp_t ataOperation = {
  NULL, NULL, NULL,
  NULL, ATA_Read, NULL, NULL,
  NULL, ATA_Write, NULL, NULL, NULL,
};

// ===========================================================================
void ATA_Initialize (kInode_t* dev)
{
  int i;
  time_t now = time(NULL);
  kStat_t stat = { 0, S_IFBLK | 0755, 0, 0, 0L, 0L, now, now, now, 0, 0, 0 };
  const char* name[] = { "sdA", "sdB", "sdC", "sdD" };

  sdx[0]._pbase = 0x1f0;
  sdx[0]._pctrl = 0x3f6;
  sdx[0]._disc = 0xa0;
  sdx[1]._pbase = 0x1f0;
  sdx[1]._pctrl = 0x3f6;
  sdx[1]._disc = 0xb0;
  sdx[2]._pbase = 0x170;
  sdx[2]._pctrl = 0x376;
  sdx[2]._disc = 0xa0;
  sdx[3]._pbase = 0x170;
  sdx[3]._pctrl = 0x376;
  sdx[3]._disc = 0xb0;

  outb(0x3f6 + ATA_REG_CONTROL - 0x0A, 2);
  outb(0x376 + ATA_REG_CONTROL - 0x0A, 2);

  for (i = 0; i < 4; ++i) {
    printf ("%s: ", name[i]);
    if (ATA_Detect(&sdx[i])) {
      int block = sdx[i]._type == IDE_ATA ? 512 : 2048;
      stat.dblock_ = stat.cblock_ = block;
      kfs_new_device (name[i], dev, &ataOperation, &sdx[i], &stat);
    }
  }
}


