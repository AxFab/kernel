#include <kernel/inodes.h>

#define TAR_RECORDSIZE 512

int KRP_Mount (kInode_t* dev, kInode_t* mnt);
int KRP_Write();
int KRP_Lookup(const char* name, kInode_t* dir, kStat_t* file);
int KRP_Read(kInode_t* fp, void* buffer, size_t count, size_t lba);

kFileOp_t krpOps = {
  KRP_Mount, NULL, NULL,
  KRP_Lookup, KRP_Read, NULL, NULL,
  (void*)KRP_Write, (void*)KRP_Write, NULL, (void*)KRP_Write, NULL,
  NULL,
};


const char* krpPrefix = "./";
extern char krpPack;
extern int krpLength;
// int krpLength;
char* krpBase;

// ===========================================================================
int KRP_Mount (kInode_t* dev, kInode_t* mnt)
{
  time_t now = time(NULL);
  kStat_t root = { 0, S_IFDIR | 0500, 0, 0, 0L, 0L, now, now, now, 0, 0, 0 };

  krpBase = &krpPack;
  kfs_new_device ("krp", mnt, &krpOps, (void*)NULL, &root);
  return __noerror ();
}


// ---------------------------------------------------------------------------
int KRP_Write()
{
  return EROFS;
}


// ---------------------------------------------------------------------------
int KRP_Lookup(const char* name, kInode_t* dir, kStat_t* file)
{
  time_t now = time(NULL);
  char uri [PATH_MAX];
  char search [PATH_MAX];
  kfs_puri (dir, uri, PATH_MAX);

  if (strlen (strchr(uri, ':') + 1) == 0)
    snprintf(search, PATH_MAX, "%s%s", krpPrefix, name);
  else
    snprintf(search, PATH_MAX, "%s%s/%s", krpPrefix, (strchr (uri, ':') + 2), name);

  if (KLOG_FS) printf ("krpFs] Look for %s -> '%s' on dir %s \n", name, search, uri);
  file->uid_ = 0;
  file->gid_ = 0;
  file->atime_ = now;
  file->mtime_ = now;
  file->ctime_ = now;
  file->dblock_ = 512;
  file->cblock_ = 512;
  int pos = 0;
  int lg = strlen (search);
  char* table = krpBase;

  while (pos < krpLength) {
    // printf ("krpFs] Compare {%s|%s} at %d \n", search, table, pos/512);
    if (memcmp(search, table, lg) == 0) {
      if (table[lg] == '/') {
        file->mode_ = S_IFDIR | 0500;
        file->length_ = 0;
        file->lba_ = 0;
        return 0;

      } else if (table[lg] == '\0') {
        file->mode_ = S_IFREG | 0500;
        file->length_ = strtol(&table[0x7c], NULL, 0);
        file->lba_ = pos;
        if (KLOG_FS) printf ("krpFs] Find %s{%s} at %d \n", name, table, pos/512);
        return 0;
      }
    }

    int sz = strtol(&table[0x7c], NULL, 0);
    sz = TAR_RECORDSIZE + ALIGN_UP(sz, TAR_RECORDSIZE);
    table += sz;
    pos += sz;
  }

  if (KLOG_FS) printf ("krpFs] Unable to find entry %s\n", search);
  return ENOENT;
}


// ---------------------------------------------------------------------------
int KRP_Read(kInode_t* fp, void* buffer, size_t count, size_t lba)
{
  size_t idx = fp->stat_.lba_ + (lba + 1) * 512;
  if (KLOG_FS) printf ("krpFs] Read file at %d[%x] for %d cluster on [%x]\n", idx, &krpBase[0], count, buffer);
  memcpy (buffer, &krpBase[idx], count * 512);

  // kTty_HexDump (&krpBase[idx], 0x50);
  return 0;
}


