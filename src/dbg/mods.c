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
  kFs_Initialize ();
  kVma_Initialize ();

  // Build Kernel file tree
  kStat_t rDir = { 0, S_IFDIR | 0555, ROOT_UID, ROOT_UID, 0, 0, 0L, 0L, 0L, 0 };
  kInode_t* devIno = kFs_MkNode("dev", kFs_RootInode(), &rDir);
  kInode_t* mntIno = kFs_MkNode("mnt", kFs_RootInode(), &rDir);
  kInode_t* tmpIno = kFs_MkNode("tmp", kFs_RootInode(), &rDir);

  // Initialize devices
  img_Initialize (devIno, mntIno); // REPLACE BY ATA !?

  // TESTING 
  kInode_t* bbx = kFs_LookFor ("/mnt/cdrom/USR/BIN/BUZYBOX.", NULL);
  kFs_Open (bbx);
  // memset (buffer, 0, 2048);
  // kFs_Feed (bbx, buffer, 0, 2048);
  kAssembly_t* bbAsm = kAsm_Open (bbx);
  kFs_Close (bbx);

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

