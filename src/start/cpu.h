#ifndef CPU_H__
#define CPU_H__

#include <kcore.h>

// #error  WHAT !?

void kCpu_DisplayRegs (kCpuRegs_t* regs);

int kCpu_Initialize ();
int kCpu_Launch ();


#endif /* CPU_H__ */
