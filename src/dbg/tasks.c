#include <tasks.h>
#include <inodes.h>
#include <stdio.h>
#include <time.h>

/*
*/

kInode_t* kFs_Lookup(char* path, kInode_t* dir)
{
  return (kInode_t*)fopen (path, "r");
}


int kFs_Open(kInode_t* ino) { return 0; }
int kFs_Close(kInode_t* ino) { return 0; }

int kFs_Read(kInode_t* ino, void* buffer, off_t offset, size_t count)
{
  fseek ((FILE*)ino, offset, SEEK_SET);
  fread(buffer, count, 1, (FILE*)ino);
  return 0;
}

int main ()
{
  kSch_Initialize();

  // Start first logon
  kInode_t* inoShell = kFs_Lookup ("bin/i686/debug/inodes", NULL);
  kAssembly_t* asmShell = kAsm_Open (inoShell);

  // kInode_t* inoHome = kFs_Lookup ("/home/", NULL);
  // kProcess_t* pFirst = kTsk_NewProcess (asmShell, inoHome);

  // Start log on program
  // kTsk_NewProcess (asmShell, inoHome);

  return 0;
}

