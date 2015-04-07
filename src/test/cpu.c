#include <kernel/core.h>

void cpu_halt ()
{
}

void cpu_start ()
{
}

void cpu_switch ()
{
}

void cpu_save () // DEPRECATED
{
}


void kCpu_Halt ()
{
}
void kCpu_Switch2 ()
{
}
int kCpu_SetStatus (int state)
{
}

void kCpu_Reset (kCpuRegs_t *regs, uintptr_t entry, uintmax_t param, uintptr_t stack)
{
}

void kCpu_Switch (kCpuRegs_t *regs, uint32_t *dir, uint32_t kstack)
{
}

void kCpu_Save (kThread_t *task, kCpuRegs_t *regs)
{
}



