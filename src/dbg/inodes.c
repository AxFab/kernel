#include <inodes.h>


int img_Initialize (kInode_t* dev, kInode_t* mnt);

int main ()
{
  char buffer[2048];

  kFs_Initialize();

  kStat_t rDir = { 0, S_IFDIR | 0555, ROOT_UID, ROOT_UID, 0, 0, 0L, 0L, 0L, 0 };

  kInode_t* devIno = kFs_MkNode("dev", kFs_RootInode(), &rDir);
  kInode_t* mntIno = kFs_MkNode("mnt", kFs_RootInode(), &rDir);

  img_Initialize (devIno, mntIno); // REPLACE BY ATA !?

  kInode_t* iso = kFs_LookFor ("/dev/iso", NULL);

  kFs_Feed(iso, buffer, 16 * 2048, 2048);
  // kdump (buffer, 2048);

  kFs_Feed(iso, buffer, 17 * 2048, 2048);
  // kdump (buffer, 2048);

  kInode_t* kimg = kFs_LookFor ("/mnt/cdrom/SYS/KERNEL.IMG", NULL);

  kFs_Feed(kimg, buffer, 0, 2048);
  kdump (buffer, 2048);

  kFs_PrintAll ();
  NO_LOCK;
  return 0;
}



