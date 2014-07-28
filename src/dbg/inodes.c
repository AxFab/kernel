#include <kernel/inodes.h>


int img_Initialize (kInode_t* dev, kInode_t* mnt);

int main ()
{
  char buffer[2048];

  kfs_init();

  kStat_t rDir = { 0, S_IFDIR | 0555, ROOT_UID, ROOT_UID, 0, 0, 0L, 0L, 0L, 0 };

  kInode_t* mntIno = kfs_mknod("mnt", kFs_RootInode(), &rDir);
  kInode_t* devIno = kfs_mknod("dev", kFs_RootInode(), &rDir);

  kfs_mknod("sys", kFs_RootInode(), &rDir);
  kfs_mknod("sys", kFs_RootInode(), &rDir);

  kfs_mknod("bin", kFs_RootInode(), &rDir);
  kfs_mknod("lib", kFs_RootInode(), &rDir);
  kfs_mknod("usr", kFs_RootInode(), &rDir);

  img_Initialize (devIno, mntIno); // REPLACE BY ATA !?

  kInode_t* iso = kfs_lookup ("/dev/iso", NULL);

  kFs_Feed(iso, buffer, 16 * 2048, 2048);
  // kdump (buffer, 2048);

  kFs_Feed(iso, buffer, 17 * 2048, 2048);
  // kdump (buffer, 2048);

  kInode_t* kimg = kfs_lookup ("/mnt/cdrom/SYS/KERNEL.IMG", NULL);

  kfs_lookup ("cdrom:\\BOOT\\GRUB\\I386_PC/FAT.MOD", NULL);
  kfs_lookup ("cdrom:/BOOT/GRUB/I386_PC/FIT.MOD", NULL);

  kfs_grab (kimg);

  kfs_plink (kimg, buffer, 2048);
  kprintf ("Found - %s \n", buffer);

  kfs_puri (kimg, buffer, 2048);
  kprintf ("Found - %s \n", buffer);

  kFs_Feed(kimg, buffer, 0, 2048);
  kdump (buffer, 2048);

  kfs_release (kimg);

  kfs_log_all ();
  NO_LOCK;
  return 0;
}



