#ifndef CPU_H__
#define CPU_H__

#include <kernel/core.h>

// #error  WHAT !?

void kregisters (kCpuRegs_t* regs);

int kCpu_Initialize ();
int kCpu_Launch ();


#endif /* CPU_H__ */
