#include <kernel/core.h>
#include <kernel/vfs.h>

#include <stdio.h>
#include <stdlib.h>


// ---------------------------------------------------------------------------
/** Function to called to grab an inodes */
int inode_open(kInode_t* ino)
{
  ++ino->readers_;
  return __seterrno(0);
}


// ---------------------------------------------------------------------------
/** Function to release an inodes */
int inode_close(kInode_t* ino)
{
  --ino->readers_;
  return __seterrno(0);
}


// ---------------------------------------------------------------------------
/** Find a physique page for the content of an inode. */
int inode_page(kInode_t* ino, off_t offset, page_t* page)
{
  if ((size_t)offset >= ino->stat_.length_) 
    return __seterrno (EINVAL);

  offset = ALIGN_DW (offset, PAGE_SIZE);

  *page = ino->pagesCache_->phys_ + offset;
  return 0;
}


// ---------------------------------------------------------------------------
kInode_t* load_inode (const char* file)
{
  FILE* fp = fopen(file, "r");
  if (!fp)
    return NULL;

  kInode_t* ino = KALLOC(kInode_t);
  fseek(fp, 0, SEEK_END);
  size_t lg = ftell(fp); 
  ino->stat_.length_ = lg;
  fseek (fp, 0, SEEK_SET);
  void* pg = malloc (lg);
  fread(pg, lg, 1, fp);
  kBucket_t* buck = KALLOC(kBucket_t);
  buck->phys_ = (page_t)pg;
  ino->pagesCache_ = buck;
  fclose(fp);
  return ino;
}


// ---------------------------------------------------------------------------
kInode_t* build_inode_dir ()
{
  kInode_t* ino = KALLOC(kInode_t);
  ino->stat_.mode_ = 0755 | S_IFDIR;
  return ino;
}

