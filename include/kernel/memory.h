#ifndef MEMORY_H__
#define MEMORY_H__

#include <kernel/core.h>
#include <kernel/aatree.h>
#include <kernel/mmu.h>



// ============================================================================



struct kVma {
  size_t       base_;
  size_t       limit_;
  size_t       length_;

  kVma_t*         next_;
  kVma_t*         prev_;
  aanode_t        bbNode_;

  int             flags_;
  kInode_t*       ino_;
  off_t           offset_;


};

struct kAddSpace 
{
  size_t phyPages_;
  size_t vrtPages_;
  spinlock_t lock_;

  kVma_t* first_;
  kVma_t* last_;

  // TODO use BBtree !
  aatree_t        bbTree_;
};


// ============================================================================



// MEMORY/ADDSPACE ===========================================================
/** Initialize a new address space structure with a first user-stack */
int addspace_init(kAddSpace_t* space, int flags);
/** Find the area holding an address */
kVma_t* addspace_find(kAddSpace_t* mspace, size_t address);

void addspace_display (kAddSpace_t* mspace);

// MEMORY/VMAREA =============================================================
/** Will allocate a new segment on the address space */
kVma_t* vmarea_map (kAddSpace_t* mspace, size_t length, int flags);
kVma_t* vmarea_map_at (kAddSpace_t* mspace, size_t address, size_t length, int flags);
kVma_t* vmarea_map_section (kAddSpace_t* mspace, kSection_t* sector, kInode_t* ino);
int vmarea_map_ino (kVma_t* area, kInode_t* ino, size_t offset);
int vmarea_grow (kAddSpace_t* mspace, kVma_t* area, size_t extra_size);
void vmarea_unmap_area (kAddSpace_t* mspace, kVma_t* area);
void vmarea_unmap (kAddSpace_t* mspace, size_t address, size_t length);

// MEMORY/PAGE ===============================================================
int page_fault (size_t address, int cause) ;




void* mmu_temporary (page_t* pg);

#endif /* MEMORY_H__ */
