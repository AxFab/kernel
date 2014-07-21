#ifndef CPU_H__
#define CPU_H__

#include <kcore.h>

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

void kCpu_DisplayRegs (kCpuRegs_t* regs);

int kCpu_Initialize ();
int kCpu_Launch ();


#endif /* CPU_H__ */
