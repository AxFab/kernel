#ifndef _KLIB_MEMORY_H
#define _KLIB_MEMORY_H 1

#include <sys/types.h>
#include <skc/bbtree.h>
#include <skc/mcrs.h>


#define MEMSP_KERNELSPACE (1 << 0)
#define MEMSP_USERSPACE (1 << 1)

#define MEM_VMA_LENGTH (64 * _Mib_)


typedef struct inode inode_t;
struct inode {
  int d;
};


#define KALLOC(t) (t*)calloc(sizeof(t),1);
void* calloc(size_t size, size_t nbr);

typedef struct memspace memspace_t;
typedef struct memvmap memvmap_t;

struct memspace {
  size_t lower_;
  size_t upper_;
  size_t virt_size_;
  size_t phys_size_;
  bbtree_t mmaps_;
  int flags_;
};

struct memvmap {
  bbnode_t node_;
  size_t length_;
  inode_t *ino_;
  off_t off_;
  int flags_;
};


void Memory_bound(memspace_t *mspace, int type);

memspace_t *Memory_create();
int Memory_append(memspace_t *mspace, void* address, size_t length, inode_t *ino, off_t offset, int flags, memvmap_t **pVma);
int Memory_split(memspace_t * mspace, void *address, size_t length, memvmap_t **pVma);


#endif /* _KLIB_MEMORY_H */
