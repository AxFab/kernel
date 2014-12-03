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
#include <kernel/vfs.h>
#include <kernel/memory.h>
#include "assembly.h"
#include "elf.h"


// ===========================================================================
/** Read a section entry on a ELF image file.
 *  If needed, a new kSection_t object is allocaed and push on assembly.
 */
static void elf_read_section (kAssembly_t* assembly, kInode_t* ino, ELF_PhEntry_t* phe)
{
  kSection_t* sec;
  // char* interpret;

  switch (phe->type_) {
    case PT_LOAD:
      sec = KALLOC (kSection_t);
      sec->address_ = phe->virtAddr_;
      sec->length_ = phe->memSize_;
      sec->align_ = phe->align_;
      sec->offset_ = phe->fileAddr_;
      sec->next_ = assembly->section_;
      sec->flags_ = phe->flags_ & 7;
      assembly->section_ = sec;
      break;

    default:
      break;
  }
}


// ---------------------------------------------------------------------------
/** Read an ELF image file and create the corresponding assembly.
 * FIXME change error code
 * FIXME choose a better reading method
 * FIXME log messages
 * TODO think about loading extra information
 */
kAssembly_t* elf_open (kInode_t* ino)
{
  int i;
  int err = 0;
  ELF_Header_t* head;
  ELF_PhEntry_t* phe;
  uint32_t page;
  kAssembly_t* assembly;

  __noerror();
  if (ino->assembly_) return ino->assembly_;

  // head = (ELF_Header_t*)kalloc(4096);
  inode_open (ino);
  inode_page (ino, 0, &page);
  // kfs_feed (ino, head, 4096 / ino->stat_.cblock_, 0);
  head = (ELF_Header_t*)mmu_temporary (&page);
  if (0) {
    kprintf ("elf] Read first page failed\n");
    // kfree(head);
    inode_close (ino);
    return NULL;
  }

  if (memcmp(ELFident, head, 16) != 0)
    err = ENOEXEC;
  else if (head->type_ != ET_EXEC)
    err = ENOEXEC;

  if (err) {
    kprintf ("elf] File is not a valid ELF file.\n");
    __seterrno (err);
    inode_close (ino);
    // kfree(head);
    return NULL;
  }

  assembly = KALLOC(kAssembly_t);
  assembly->entry_ = (void*)head->entry_;
  for (i = 0; i < head->phCount_; ++i) {
    size_t off = head->phOff_ + i * sizeof(ELF_PhEntry_t);
    assert (off + sizeof(ELF_PhEntry_t) < 4096);
    phe = (ELF_PhEntry_t*)((size_t)head + off);

    elf_read_section (assembly, ino, phe);
  }

  if (!assembly->section_) {
    kprintf ("elf] File doesn't have loadable section\n");
    kasm_destroy (assembly);
    __seterrno (ENOEXEC);
    inode_close (ino);
    // kfree(head);
    return NULL;
  }

  ino->assembly_ = assembly;
  return assembly;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
