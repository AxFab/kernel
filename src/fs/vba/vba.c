#include <kernel/inodes.h>


int VBA_Read (kInode_t* ino, void* bucket, size_t count, size_t lba);
int VBA_Write (kInode_t* ino, void* bucket, size_t count, size_t lba);
uint32_t VBA_map (kInode_t* fp, off_t offset);


kDevice_t vbaOperation = {
  0, {0}, NULL, 
  NULL, NULL, NULL, NULL,
  NULL, NULL,
  VBA_map
};


void *vbaAddress = NULL;
int vbaWidth;
int vbaHeight;
int vbaDepth;

void VBA_Set (void* address, int width, int height, int depth)
{
  vbaAddress = address;
  vbaWidth = width;
  vbaHeight = height;
  vbaDepth = depth;
}


void VBA_Initialize (kInode_t* dev)
{
  if (vbaAddress == NULL)
    return;

  size_t lg = vbaWidth * vbaHeight * vbaDepth;
  size_t line = vbaWidth * vbaDepth;

  kStat_t stat = { 0 };
  stat.mode_ = S_IFBLK | 0700;
  stat.atime_ = stat.ctime_ = stat.mtime_ = time (NULL);
  stat.length_ = lg;
  stat.lba_ = (size_t)vbaAddress;
  stat.block_ = line;
  
  // kStat_t stat = { 0, , 0, 0, lg, , now, now, now, 0, vbaDepth, line };
  kfs_new_device ("vba", dev, &vbaOperation, NULL, &stat);
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
  // kTty_HexDump (bucket, 0x20);
  kprintf ("vba] write %x, %d, %d\n", bucket, count, lba);
  size_t line = ino->stat_.block_;
  uint32_t pixels = (uint32_t)vbaAddress;
  pixels += lba * line;
  memcpy ((void*)pixels, bucket, count * line);
  // kTty_HexDump (vbaAddress, 0x20);
  return 0;
}

uint32_t VBA_map (kInode_t* fp, off_t offset)
{
  if (offset < 0 || offset >= vbaWidth * vbaHeight * vbaDepth)
    return 0;
  offset = ALIGN_DW(offset, PAGE_SIZE);
  return (uint32_t)vbaAddress + offset;
}

