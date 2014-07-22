#ifndef ASSEMBLY_H__
#define ASSEMBLY_H__

#include <kcore.h>

// ============================================================================

struct kAssembly {
  void* entry_;
  size_t stackSize_;
  kSection_t* section_;
};

// ============================================================================


// ASSEMBLY
kAssembly_t* kAsm_Open (kInode_t* ino);



#endif /* ASSEMBLY_H__ */
