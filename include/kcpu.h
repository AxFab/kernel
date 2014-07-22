#ifndef KCPU_H__
#define KCPU_H__

#include <kcore.h>

int32_t atomic_xchg_i32 (int32_t* ref, int32_t val);
int32_t atomic_inc_i32 (int32_t* ref);
int32_t atomic_dec_i32 (int32_t* ref);
int32_t atomic_add_i32 (int32_t* ref, int32_t val);

void outb (uint16_t port, uint8_t value);
void outw (uint16_t port, uint16_t value);
uint8_t inb (uint16_t port);
uint16_t inw (uint16_t port);

void insl (uint16_t port, void* addr, uint32_t count);
void insw (uint16_t port, void* addr, uint32_t count);
void outsw (uint16_t port, void* addr, uint32_t count);




struct kCpuRegs 
{
  uint16_t    gs, rs4;
  uint16_t    fs, rs3;
  uint16_t    es, rs2;
  uint16_t    ds, rs1;
  uint32_t    edi;
  uint32_t    esi;
  uint32_t    ebp;
  uint32_t    espx;
  uint32_t    ebx;
  uint32_t    edx;
  uint32_t    ecx;
  uint32_t    eax;
  uint32_t    eip;
  uint16_t    cs, rs0;
  uint32_t    eflags;
  uint32_t    esp;
  uint16_t    ss, rs5;
};

void kCpu_Reset (kCpuRegs_t* regs, uintptr_t entry, uintmax_t param);
void kCpu_Switch (kCpuRegs_t* regs, uint32_t* dir);




#endif /* KCPU_H__ */
