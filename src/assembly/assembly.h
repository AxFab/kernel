/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Extract assembly information form an ELF file.
 */
#ifndef _ASSEMBLY_H__
#define _ASSEMBLY_H__
#include <kernel/assembly.h>


// ===========================================================================
/** The structure hold information about the segment of an executable image.
 */
struct kSection
{
  uintptr_t   address_;
  uintptr_t   length_;
  uintptr_t   align_;
  uintptr_t   offset_;
  kSection_t* next_ ;
  int         flags_;
};


// ---------------------------------------------------------------------------
/** This structure hold all the information about an executable image
 */
struct kAssembly {
  void* entry_;
  size_t stackSize_;
  kSection_t* section_;
};


// ===========================================================================

kAssembly_t* elf_open (kInode_t* ino);


#endif /* _ASSEMBLY_H__ */
