#ifndef MEMORY_H__
#define MEMORY_H__

#include <kernel/core.h>


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

void kvma_init (void);
kAddSpace_t* kvma_new (size_t stack_size);
kAddSpace_t* kvma_clone (kAddSpace_t* addp);
int kvma_destroy (kAddSpace_t* addp);

kVma_t* kvma_mmap (kAddSpace_t* addressSpace, kVma_t* area);
int kvma_unmap (kAddSpace_t* addp, uintptr_t address, size_t length);
int kvma_grow_up (kAddSpace_t* addp, void* address, size_t extra_size);
int kvma_grow_down (kAddSpace_t* addp, void* address, size_t extra_size);

kVma_t* kvma_look_at (kAddSpace_t* addp, uintptr_t address);
kVma_t* kvma_look_ino (kAddSpace_t* addp, kInode_t* ino, off_t offset);
void kvma_display(kAddSpace_t* addp);

void kpg_dump (uint32_t *table);
void kpg_resolve (uint32_t address, uint32_t *table, int rights, int dirRight, uint32_t page, int reset);
int kpg_fault (uint32_t address);
uint32_t kpg_new ();

void kpg_init (void);
uintptr_t kpg_alloc (void);
void kpg_release (uintptr_t page);
void kpg_ram (uint64_t base, uint64_t length);

int kpg_fill_stream (kVma_t* vma, uint32_t address, int rights);
void kpg_sync_stream (kVma_t* vma, uint32_t address);

void* kpg_temp_page (uint32_t* pg);

#endif /* MEMORY_H__ */
