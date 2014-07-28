#include <inodes.h>


int VBA_Read (kInode_t* ino, void* bucket, size_t count, size_t lba);
int VBA_Write (kInode_t* ino, void* bucket, size_t count, size_t lba);

kFileOp_t vbaOperation = {
  NULL, NULL, NULL,
  NULL, VBA_Read, NULL, NULL,
  NULL, VBA_Write, NULL, NULL, NULL,
};


void *vbaAddress = NULL;
int vbaWidth;
int vbaHeight;
int vbaDepth;

void VBA_Set (void* address, int width, int height, int depth)
{
  vbaAddress = 0x0010000;
  vbaWidth = width;
  vbaHeight = height;
  vbaDepth = depth;
}


void VBA_Initialize (kInode_t* dev)
{
  if (vbaAddress == NULL)
    return;

  time_t now = time(NULL);
  size_t lg = vbaWidth * vbaHeight * vbaDepth;
  size_t line = vbaWidth * vbaDepth;
  kStat_t stat = { 0, S_IFBLK | 0700, 0, 0, lg, (size_t)vbaAddress, now, now, now, 0, vbaDepth, line };
  kFs_CreateDevice ("vba", dev, &vbaOperation, NULL, &stat);
}



int VBA_Read (kInode_t* ino, void* bucket, size_t count, size_t lba)
{
  size_t line = vbaWidth * vbaDepth;
  uint32_t pixels = (uint32_t)vbaAddress;
  pixels += lba * line;
  memcpy (bucket, (void*)pixels, count * line);
  return 0;
}

int VBA_Write (kInode_t* ino, void* bucket, size_t count, size_t lba)
{
  kTty_HexDump (bucket, 0x20);
  kprintf ("vba] write %x, %d, %d\n", bucket, count, lba);
  size_t line = ino->stat_.cblock_;
  uint32_t pixels = (uint32_t)vbaAddress;
  pixels += lba * line;
  memcpy ((void*)pixels, bucket, count * line);
  kTty_HexDump (vbaAddress, 0x20);
  return 0;
}

