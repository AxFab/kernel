#include <kernel/files.h>
#include <kernel/fs.h>
#include <stdio.h>
// #include <unistd.h>

#define TAR_RECORDSIZE 512

long strtol(const char*, const char**, int);
ssize_t kreadlink(kStat_t* dir, char* buf, size_t bufsiz);
off_t lseek(int fd, off_t offset, int whence);
ssize_t read(int fd, void* buf, size_t count);

int krpMount(int dev, kStat_t* root);
int krpUnmount(void);

int krpLookup(const char* name, kStat_t* dir, kStat_t* file);
int krpRead (kStat_t* fp, void* bucket, size_t offset, size_t length);

// ===========================================================================
const char* krpPrefix = "krp/";
extern char krpPack;
extern int krpLength;
// int krpLength;
char* krpBase;

kVolume_t krpFs = {
  "KRP'FS",
  &krpMount, &krpUnmount, NULL,
  &krpLookup, &krpRead, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
};


// ===========================================================================
int krpMount(int dev, kStat_t* root)
{
  time_t now = ktime();
  root->mode_ = S_IFDIR | 0500;
  root->uid_ = ROOT_UID;
  root->gid_ = ROOT_UID;
  root->atime_ = now;
  root->mtime_ = now;
  root->ctime_ = now;
  // if (dev == 0) {
  krpBase = &krpPack;
  // } else {
  //   // krpLength = lseek(dev, 0, SEEK_END);
  //   // krpBase = malloc (krpLength);
  //   // lseek (dev, 0, SEEK_SET);
  //   // read (dev, krpBase, krpLength);
  // }
  return 0;
}

int krpUnmount(void)
{
  free (krpBase);
  return 0;
}


// statfs / options




// ===========================================================================
int krpLookup(const char* name, kStat_t* dir, kStat_t* file)
{
  time_t now = ktime();
  char uri [PATH_MAX];
  char search [PATH_MAX];
  kreadlink (dir, uri, PATH_MAX);

  if (strlen (strchr(uri, ':') + 1) == 0)
    snprintf(search, PATH_MAX, "%s%s", krpPrefix, name);

  else
    snprintf(search, PATH_MAX, "%s%s/%s", krpPrefix, (strchr (uri, ':') + 2), name);

  // printf ("krpFs] Look for %s -> '%s' on dir %s \n", name, search, uri);
  file->uid_ = ROOT_UID;
  file->gid_ = ROOT_UID;
  file->atime_ = now;
  file->mtime_ = now;
  file->ctime_ = now;
  int pos = 0;
  int lg = strlen (search);
  char* table = krpBase;

  while (pos < krpLength) {
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
        return 0;
      }
    }

    int sz = strtol(&table[0x7c], NULL, 0);
    sz = TAR_RECORDSIZE + ALIGN_UP(sz, TAR_RECORDSIZE);
    table += sz;
    pos += sz;
  }

  printf ("krpFs] Unable to find entry %s\n", search);
  return ENOENT;
}


int krpRead (kStat_t* fp, void* bucket, size_t offset, size_t length)
{
  char* table = krpBase + (uintptr_t)fp->lba_ + 512 + offset;
  // printf ("Copy %d bytes from %x to %x\n", length, table, bucket);
  // kTty_HexDump (table, 128);
  // TODO Add position checks and fp validity
  memcpy (bucket, table, length);
  // kTty_HexDump (bucket, 128);
  return length;
}


// int (*readdir)();
// int (*readlink)();

