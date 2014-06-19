#ifndef KINFO_H__
#define KINFO_H__

#include <kcore.h>

typedef struct kCpuCore kCpuCore_t;
struct kCpuCore {
  // CORE
  int   errNo;
  int   lockCounter;

  // INODES
  // RootInode

  // MEMORY
  // AddressSpace
};

typedef struct kSysCore kSysCore_t;
struct kSysCore {
  // xHeapArea_t   kheap;
  kInode_t* RootFs;
};

extern kCpuCore_t kCPU;
extern kSysCore_t kSYS;

#endif /* KINFO_H__ */
