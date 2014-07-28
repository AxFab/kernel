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
#include "assembly.h"
#include <kernel/inodes.h>
#include <kernel/memory.h>


// ===========================================================================
/**
 */
void kasm_destroy (kAssembly_t* assembly)
{
  kSection_t* sec;

  while (assembly->section_) {
    sec = assembly->section_->next_;
    kfree(assembly->section_);
    assembly->section_ = sec;
  }

  kfree(assembly);
}


// ---------------------------------------------------------------------------
/** Load an assembly image on an address space.
 *  Basically, it place the correct virtual memory area at the right addresses.
 *  \note A nice feature will be to compute offset for relocation.
 */
int kasm_load (kAddSpace_t* mmsp, kInode_t* ino)
{
  kAssembly_t* assembly = ino->assembly_;
  kVma_t area = { VMA_EXECUTABLE, 0L, 0L, 0, 0, ino, 0 };

  kSection_t* sec = assembly->section_;
  while (sec != NULL) {
    area.base_ = sec->address_;
    area.limit_ = area.base_ + sec->length_;
    area.offset_ = sec->offset_;
    kVma_MMap (mmsp, &area);
    sec = sec->next_;
  }

  return __noerror();
}


// ---------------------------------------------------------------------------
/** Read an image file and create the corresponding assembly.
 */
kAssembly_t* kasm_open (kInode_t* ino)
{
  return elf_open (ino);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
