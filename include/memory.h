#ifndef MEMORY_H__
#define MEMORY_H__

#include <kcore.h>


#define VMA_WRITE         0x002 ///< Pages can be written to
#define VMA_EXEC          0x001 ///< Pages can be executed
#define VMA_READ          0x004 ///< Pages can be read from
#define VMA_SHARED        0x008 ///< Page are shared
#define VMA_MAYREAD       0x010 ///< The VMA_READ flag can be set
#define VMA_MAYWRITE      0x020 ///< The VMA_WRITE flag can be set
#define VMA_MAYEXEC       0x040 ///< The VMA_EXEC flag can be set
#define VMA_MAYSHARED     0x080 ///< The VMA_SHARED flag can be set
#define VMA_GROWSDOWN     0x100 ///< The area can grow downward
#define VMA_GROWSUP       0x200 ///< The area can grow upward

#define VMA_SHM           0x400 ///< The area is used for shared memory
#define VMA_EXECUTABLE    0x800 ///< The area map an executable file
#define VMA_HEAP          0x1000 ///< The area map a process heap
#define VMA_STACK         0x2000 ///< The area map a thread stack

#define VMA_CODE          0x4000
#define VMA_DATA          0x8000

// #define VMA_COPY          0x008 ///< The page need to be copied before writting


// ============================================================================



struct kVma {
  int             flags_;
  uintptr_t       base_;
  uintptr_t       limit_;
  kVma_t*         next_;
  kVma_t*         prev_;
  kInode_t*       ino_;
  off_t           offset_;
};

struct kAddSpace {
  kVma_t* first_;
  kVma_t* last_;
  size_t vrtPages_;
  size_t phyPages_;
  spinlock_t lock_;
};


// ============================================================================

int kVma_Initialize ();
kAddSpace_t* kVma_New (size_t stack_size);
kAddSpace_t* kVma_Clone (kAddSpace_t* addp);
int kVma_Destroy (kAddSpace_t* addp);

kVma_t* kVma_MMap (kAddSpace_t* addressSpace, kVma_t* area);
int kVma_Unmap (kAddSpace_t* addp, uintptr_t address, size_t length);

kVma_t* kVma_FindAt (kAddSpace_t* addp, uintptr_t address);
kVma_t* kVma_FindFile (kAddSpace_t* addp, uintptr_t address, kInode_t* ino, off_t offset);
void kVma_Display(kAddSpace_t* addp);

int kVma_GrowUp (kAddSpace_t* addp, void* address, size_t extra_size);
int kVma_GrowDown (kAddSpace_t* addp, void* address, size_t extra_size);


#endif /* MEMORY_H__ */
