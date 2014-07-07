#ifndef KINFO_H__
#define KINFO_H__

#include <kcore.h>
#ifdef __KERNEL
#  include <alloc.h>
#endif

typedef struct kCpuCore kCpuCore_t;
struct kCpuCore 
{
  // CORE
  int   errNo;
  int   lockCounter;

  // INODES
  // RootInode

  // MEMORY
  // AddressSpace
};

typedef struct kSysCore kSysCore_t;
struct kSysCore 
{
  
  kInode_t* RootFs;

#ifdef __KERNEL
  xHeapArea_t   kheap;
#endif
};

extern kCpuCore_t kCPU;
extern kSysCore_t kSYS;

#endif /* KINFO_H__ */
