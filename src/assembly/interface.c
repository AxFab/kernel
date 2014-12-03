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
#include <kernel/vfs.h>
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
int kasm_load (kAddSpace_t* mspace, kInode_t* ino)
{
  kSection_t* section;
  kAssembly_t* assembly = ino->assembly_;
  kVma_t area = {0};
  area.flags_ = VMA_FILE;
  area.ino_ = ino;

  section = assembly->section_;
  while (section != NULL) {
    area.base_ = section->address_;
    area.limit_ = area.base_ + section->length_;
    area.offset_ = section->offset_;
    area.flags_ = (section->flags_ & 7) | VMA_FILE;
    if ((section->flags_ & VMA_WRITE) == 0 )
      area.flags_ |= VMA_SHARED;
    // kprintf ("ELF : %x - %x", section->address_, section->offset_);
    assert ((section->offset_ & (PAGE_SIZE -1)) == 0);

    // kvma_mmap (mspace, &area);
    vmarea_map_section (mspace, section, ino);

    section = section->next_;
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
