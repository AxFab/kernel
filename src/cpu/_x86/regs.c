#include <smkos/kernel.h>

// void cpuid(int code, uint32_t* a, uint32_t* d)
// {
//   asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
// }


uint64_t rdtsc()
{
  uint64_t ret;
  asm volatile ( "rdtsc" : "=A"(ret) );
  return ret;
}


unsigned long read_cr0(void)
{
  unsigned long val;
  asm volatile ( "mov %%cr0, %0" : "=r"(val) );
  return val;
}


void invlpg(void* m)
{
  /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
  asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}


void wrmsr(uint32_t msr_id, uint64_t msr_value)
{
  asm volatile ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
}


uint64_t rdmsr(uint32_t msr_id)
{
  uint64_t msr_value;
  asm volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
  return msr_value;
}

