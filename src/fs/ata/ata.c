#include "ata.h"

// ===========================================================================
kVolume_t ataFs = {
  "ATA - ATAPI",
  &ataMount, NULL, NULL,
  NULL, &ataRead, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
};


// ===========================================================================
kAtaDrive_t sdx[4];

// ===========================================================================

int ataMount(int dev, kStat_t* root)
{
  time_t now = ktime();

  if (sdx[dev]._type == IDE_ATAPI)
    root->mode_ = S_IFBLK | 0500;

  else if (sdx[dev]._type == IDE_ATA)
    root->mode_ = S_IFBLK | 0700;

  root->dev_ = dev;
  root->uid_ = ROOT_UID;
  root->gid_ = ROOT_UID;
  root->atime_ = now;
  root->mtime_ = now;
  root->ctime_ = now;
  return __noerror();
}



int ataInitialize(kInode_t* dev)
{
  int i;
  const char* name[] = { "sdA", "sdB", "sdC", "sdD" };
  // kStat_t stat = { 0, S_IFBLK | 0755, ROOT_UID, ROOT_UID,
  //     0ULL, 0ULL, 0, 0, 0,
  //     0, NULL };
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

    if (ataDetect(&sdx[i])) {
      kFs_Mount (name[i], dev, &ataFs, (kInode_t*)i, 0);
    }
  }

  return 0;
}



