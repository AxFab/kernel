#ifndef ASSEMBLY_H__
#define ASSEMBLY_H__

#include <kernel/core.h>



void kasm_destroy (kAssembly_t* assembly);
int kasm_load (kAddSpace_t* mmsp, kInode_t* ino);
kAssembly_t* kasm_open (kInode_t* ino);




#endif /* ASSEMBLY_H__ */
