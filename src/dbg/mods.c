#include <kcore.h>
#include <inodes.h>
#include <memory.h>
#include <tasks.h>

void kCore_Initialize ()
{
}

int main ()
{
  char buffer[2048];

  // Initialize
  kCore_Initialize ();
  kfs_init ();
  kVma_Initialize ();

  // Build Kernel file tree
  kStat_t rDir = { 0, S_IFDIR | 0555, ROOT_UID, ROOT_UID, 0, 0, 0L, 0L, 0L, 0 };
  kInode_t* devIno = kfs_mknod("dev", kFs_RootInode(), &rDir);
  kInode_t* mntIno = kfs_mknod("mnt", kFs_RootInode(), &rDir);
  kInode_t* tmpIno = kfs_mknod("tmp", kFs_RootInode(), &rDir);

  // Initialize devices
  img_Initialize (devIno, mntIno); // REPLACE BY ATA !?

  // TESTING
  kInode_t* bbx = search_inode ("/mnt/cdrom/USR/BIN/BUZYBOX.", NULL);
  kfs_grab (bbx);
  // memset (buffer, 0, 2048);
  // kFs_Feed (bbx, buffer, 0, 2048);
  kAssembly_t* bbAsm = kAsm_Open (bbx);
  kfs_release (bbx);

  // kdump (buffer, 2048);

  /*
    READ PAGE
    WRITE PAGE
    CHAR - STREAM
  */

  // Start process
  //  - Create a vma
  //  - Put Linker on it (kernel page)
  //  - Start as kernel page (in user mode)
  //  - Look for file
  //  - Exec


  return 0;
}

