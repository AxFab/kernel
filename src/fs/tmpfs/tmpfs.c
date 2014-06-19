#include <inodes.h>

// STANDARD AVAILABLE FUNCTION
// read / write / syncfs
// printf / malloc / free
// str* / mem*
// time


int tmpfsLookup(const char* name, kInode_t* dir, kStat_t* file);
int tmpfsCreate(const char* name, kInode_t* dir, kStat_t* file);





int tmpfsLookup(const char* name, kInode_t* dir, kStat_t* file)
{
  return ENOENT;
}

int tmpfsCreate(const char* name, kInode_t* dir, kStat_t* file)
{
  return 0;
}

int tmpfsMount (kFsys_t* fs)
{
  if (fs->dev_)
    return ENXIO;

  fs->block_ = 512;
  fs->cluster_ = 512;
  fs->lookup = tmpfsLookup;
  fs->create = tmpfsCreate;
  strncpy (fs->name_, "tmpfs", 16);
  return 0;
}


