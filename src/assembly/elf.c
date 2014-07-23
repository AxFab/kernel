/**
 *      This file is part of the KERNEL project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Extract assembly information form an ELF file.
 */
#include <assembly.h>
#include <inodes.h>
#include <memory.h>
#include "elf.h"

// ---------------------------------------------------------------------------
/** The section structure hold information about a segment of an executable
 *  image.
 */
struct kSection
{
  uintptr_t   address_;
  uintptr_t   length_;
  uintptr_t   align_;
  uintptr_t   offset_;
  kSection_t* next_ ;
};

// ---------------------------------------------------------------------------
/** Destroy an assembly and release allocated memory
 *  @bug no locking mechanism have been put in place to garanted end of use.
 */
static void kAsm_Destroy (kAssembly_t* assembly)
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
 *  @note A nice feature will be to compute offset for relocation.
 *  @bug We need to handle the case were loading in part or complete failed.
 *       This is probable with shared object without reloc info.
 */
void kAsm_Load (kAddSpace_t* mmsp, kInode_t* ino)
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

  // kVma_Display (mmsp);
}


// ---------------------------------------------------------------------------
/** Read a section entry on a ELF image file.
 *  If needed, a new kSection_t object is allocaed and push on assembly.
 */
void kAsm_ReadSection (kAssembly_t* assembly, kInode_t* ino, ELF_PhEntry_t* phe)
{
  kSection_t* sec;
  char* interpret;

  switch (phe->type_) {
    case PT_LOAD:
      sec = KALLOC (kSection_t);
      sec->address_ = phe->virtAddr_;
      sec->length_ = phe->memSize_;
      sec->align_ = phe->align_;
      sec->offset_ = phe->fileAddr_;
      sec->next_ = assembly->section_;
      assembly->section_ = sec;
      break;

    default:
      break;
  }
}

// ---------------------------------------------------------------------------
/** Read an ELF image file and create the corresponding assembly.
 */
kAssembly_t* kAsm_Open (kInode_t* ino)
{
  int i;
  int err = 0;
  ELF_Header_t* head;
  ELF_PhEntry_t* phe;
  kAssembly_t* assembly;
  kSection_t* sec;

  // FIXME error code are bad choice
  __noerror();
  if (ino->assembly_) return ino->assembly_;

  head = (ELF_Header_t*)kalloc(4096);
  kFs_Open (ino);
  kFs_Read (ino, head, 4096 / ino->stat_.cblock_, 0);
  if (0) {
    kprintf ("elf] Read first page failed\n");
    kfree(head);
    kFs_Close (ino);
    return NULL;
  }

  if (memcmp(ELFident, head, 16) != 0)
    err = ENOEXEC;
  else if (head->type_ != ET_EXEC)
    err = ENOEXEC;

  if (err) {
    kprintf ("elf] File is not a valid ELF file.\n");
    __seterrno (err);
    kFs_Close (ino);
    kfree(head);
    return NULL;
  }

  assembly = KALLOC(kAssembly_t);
  assembly->entry_ = (void*)head->entry_;
  for (i = 0; i < head->phCount_; ++i) {
    size_t off = head->phOff_ + i * sizeof(ELF_PhEntry_t);
    assert (off + sizeof(ELF_PhEntry_t) < 4096);
    phe = (ELF_PhEntry_t*)((size_t)head + off);

    kAsm_ReadSection (assembly, ino, phe);
  }

  if (!assembly->section_) {
    kprintf ("elf] File doesn't have loadable section\n");
    kAsm_Destroy (assembly);
    __seterrno (ENOEXEC);
    kFs_Close (ino);
    kfree(head);
    return NULL;
  }

  ino->assembly_ = assembly;
  return assembly;
}

