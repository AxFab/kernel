/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Open and Load assembly file.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>

#include "asm_elf.h"

/* ----------------------------------------------------------------------- */
/** Read a section entry on a ELF image file.
  * If needed, a new kSection_t object is allocaed and push on assembly.
  */
static void elf_read_section (kAssembly_t *assembly, kInode_t *ino, struct ELF_phEntry *phe)
{
  kSection_t *sec;
  // char* interpret;

  switch (phe->type_) {
  case PT_LOAD:
    sec = KALLOC (kSection_t);
    sec->address_ = phe->virtAddr_;
    sec->length_ = phe->memSize_;
    sec->align_ = phe->align_;
    sec->offset_ = phe->fileAddr_;
    sec->flags_ = phe->flags_ & 7;
    ll_push_back(&assembly->sections_, &sec->node_);
    break;

  default:
    break;
  }
}


/* ----------------------------------------------------------------------- */
/** Read an ELF image file and create the corresponding assembly.
  * @todo think about loading extra information
  */
static kAssembly_t *elf_open (kInode_t *ino)
{
  int i;
  struct ELF_header *head;
  struct ELF_phEntry *phe;
  kMemArea_t *area;
  kAssembly_t *assembly;

  if (ino->assembly_)
    return ino->assembly_;

  area = area_map_ino(kSYS.mspace_, ino, 0, PAGE_SIZE, 0);
  area->at_ = __AT__;
  head = (struct ELF_header *)area->address_;

  if (head == NULL) {
    area_unmap(kSYS.mspace_, area);
    return NULL;
  }

  if (memcmp(ELFident, head, 16) != 0 || head->type_ != ET_EXEC) {
    area_unmap(kSYS.mspace_, area);
    return __seterrnoN(ENOEXEC, kAssembly_t);
  }

  assembly = KALLOC(kAssembly_t);
  assembly->entryPoint_ = (size_t)head->entry_;

  for (i = 0; i < head->phCount_; ++i) {
    size_t off = head->phOff_ + i * sizeof(struct ELF_phEntry);
    assert (off + sizeof(struct ELF_phEntry) < 4096);
    phe = (struct ELF_phEntry *)((size_t)head + off);
    elf_read_section (assembly, ino, phe);
  }

  if (assembly->sections_.count_ == 0) {
    // kprintf ("elf] File doesn't have any loadable section\n");
    destroy_assembly (assembly);
    area_unmap(kSYS.mspace_, area);
    return __seterrnoN (ENOEXEC, kAssembly_t);
  }

  ino->assembly_ = assembly;
  assembly->ino_ = ino;
  area_unmap(kSYS.mspace_, area);
  return assembly;
}


/* ----------------------------------------------------------------------- */
/** Destroy an assembly
  * @todo We need to decide who should open the inode and fix cleaning with it.
  */
void destroy_assembly (kAssembly_t *image)
{
  kSection_t *section = ll_pop_back(&image->sections_, kSection_t, node_);

  assert(image->usage_ == 0);

  while (section) {
    kSection_t *pick = section;
    section = ll_pop_back(&image->sections_, kSection_t, node_);
    kfree (pick);
  }

  kfree(image);
}


/* ----------------------------------------------------------------------- */
/** Read an image file and create the corresponding assembly.
  */
kAssembly_t *load_assembly (kInode_t *ino)
{
  assert (S_ISREG (ino->stat_.mode_));

  if (ino->assembly_ == NULL) {
    kAssembly_t *image;

    if (inode_open(ino))
      return NULL;

    image = elf_open (ino);

    if (image == NULL) {
      int err = __geterrno();
      inode_close (ino);
      __seterrno(err);
      return NULL;
    }

    image->ino_ = ino;
    ino->assembly_ = image;
  }

  return ino->assembly_;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
