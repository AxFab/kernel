#include <kernel/vfs.h>
#include <kernel/info.h>

uint32_t kpg_alloc() {
  assert (0);
  return 0;
}

int img_Initialize (kInode_t* dev, kInode_t* mnt);

int main ()
{
  char buffer[2048];

  kCpu_SetStatus (CPU_STATE_SYSCALL);
  kfs_init();

  kStat_t rDir = { 0, S_IFDIR | 0555, ROOT_UID, ROOT_UID, 0, 0, 0L, 0L, 0L, 0 };

  // kInode_t* mntIno = kfs_mknod("mnt", kSYS.rootNd_, &rDir);

  kfs_mknod("sys", kSYS.rootNd_, &rDir);
  // kfs_mknod("sys", kSYS.rootNd_, &rDir);

  kfs_mknod("bin", kSYS.rootNd_, &rDir);
  kfs_mknod("lib", kSYS.rootNd_, &rDir);
  kfs_mknod("usr", kSYS.rootNd_, &rDir);

  IMG_init (kSYS.devNd_); // REPLACE BY ATA !?

  kInode_t* iso = search_inode ("/dev/sdA", NULL);
  int err = ISO_Mount (iso, kSYS.mntNd_);
  kprintf ("ISO mount %d \n", err);

  // kfs_feed(iso, buffer, 1, 16);
  // // kdump (buffer, 2048);

  // kfs_feed(iso, buffer, 1, 17);
  // kdump (buffer, 2048);

  kInode_t* kimg = search_inode ("/mnt/OS_CORE/BOOT/KIMAGE.", NULL);
  kfs_log_all();

  // search_inode ("cdrom:\\BOOT\\GRUB\\I386_PC/FAT.MOD", NULL);
  // search_inode ("cdrom:/BOOT/GRUB/I386_PC/FIT.MOD", NULL);

  kfs_grab (kimg);

  kfs_plink (kimg, buffer, 2048);
  kprintf ("Found - %s \n", buffer);

  kfs_puri (kimg, buffer, 2048);
  kprintf ("Found - %s \n", buffer);

  feed_inode(kimg, buffer, 0, 2048);
  // kfs_feed(kimg, buffer, 0, 2048);
  kdump (buffer, 2048);

  kfs_release (kimg);

  kfs_log_all ();
  NO_LOCK;
  return 0;
}



