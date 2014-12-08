#include <kernel/core.h>
#include "../start/tty.h"
#include "../start/cpu.h"

#include "kernel/vfs.h"
#include "kernel/memory.h"
#include "kernel/scheduler.h"
#include <kernel/assembly.h>
#include <kernel/stream.h>

#define CPUID_FEATURE1_EDX_FPU   (1 << 0)
#define CPUID_FEATURE1_EDX_VME   (1 << 1)
#define CPUID_FEATURE1_EDX_PE    (1 << 2)
#define CPUID_FEATURE1_EDX_PSE   (1 << 3)
#define CPUID_FEATURE1_EDX_TSC   (1 << 4)
#define CPUID_FEATURE1_EDX_MSR   (1 << 5)
#define CPUID_FEATURE1_EDX_PAE   (1 << 6)
#define CPUID_FEATURE1_EDX_MCE   (1 << 7)
#define CPUID_FEATURE1_EDX_CX8   (1 << 8)
#define CPUID_FEATURE1_EDX_APIC  (1 << 9)
#define CPUID_FEATURE1_EDX_SEP   (1 << 11)
#define CPUID_FEATURE1_EDX_MMTR  (1 << 12)
#define CPUID_FEATURE1_EDX_PSE36 (1 << 17)
#define CPUID_FEATURE1_EDX_MMX   (1 << 23)
#define CPUID_FEATURE1_EDX_SSE   (1 << 25)
#define CPUID_FEATURE1_EDX_SSE2  (1 << 26)
#define CPUID_FEATURE1_EDX_HHT   (1 << 28)
#define CPUID_FEATURE1_EDX_TM    (1 << 29)

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi);
void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi);

#define IA32_MSR_TSC_AUX  0xC0000103
#define IA32_MSR_TSC  0x10
#define IA32_MSR_MPERF  0xE7
#define IA32_MSR_APERF  0xE78

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
 
 
/* Set the physical address for local APIC registers */
void cpuSetAPICBase(uintptr_t apic);
/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpuGetAPICBase();
 
void enableAPIC();

uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg)
{
   uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
   ioapic[0] = (reg & 0xff);
   return ioapic[4];
}
 
void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value)
{
   uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
   ioapic[0] = (reg & 0xff);
   ioapic[4] = value;
}

void cpuid(int leave, int sublv, int* cpu);

void cpu_info ()
{
  int cpuidret[5] = {0};

  cpuid(0, 0, cpuidret);
  int maxCpuidCall = cpuidret[0];
  kprintf("CPUID 0: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  kprintf("Model %s \n", (char*)(&cpuidret[1]));

  cpuid(1, 0, cpuidret);
  kprintf("CPUID 1: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  
  int stepping = cpuidret[0] & 0x7;
  int model = (cpuidret[0] >> 3) & 0xf;
  int family = (cpuidret[0] >> 7) & 0xf;
  int procType = (cpuidret[0] >> 11) & 0x3;
  int modelExt = (cpuidret[0] >> 13) & 0xf;
  int familyExt = (cpuidret[0] >> 19) & 0xff;

  int brandIndex = cpuidret[1] & 0xFF;
  int cacheSize = ((cpuidret[1] >> 8) & 0xFF) * 8;
  int maxLogProc = (cpuidret[1] >> 16) & 0xFF;
  int localAPICId = (cpuidret[1] >> 24) & 0xFF;
  
  kprintf ("Stepping: %d\n", stepping);
  kprintf ("Model: %d\n", model);
  kprintf ("Family: %d\n", family);
  kprintf ("Processor Type: %d\n", procType);
  kprintf ("Extended Model: %d\n", modelExt);
  kprintf ("Extended Family: %d\n", familyExt);

  kprintf("Brand: %d \n", brandIndex);
  kprintf("CacheLine: %d \n", cacheSize);
  kprintf("MaxLogicalProcessor: %d \n", maxLogProc);
  kprintf("APIC Id: %d \n", localAPICId);

  if (cpuidret[2] & CPUID_FEATURE1_EDX_FPU) kprintf ("  OnBoard x87 FPU\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_VME) kprintf ("  Virtual 8086 mode extensions\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_TSC) kprintf ("  Time Stamp Counter\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_MSR) kprintf ("  Model-specific registers\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_PAE) kprintf ("  Physical Address Extension\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_APIC) kprintf ("  Onboard Advanced Programmable Interrupt Controller\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_MMTR) kprintf ("  MMTR features\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_PSE36) kprintf ("  36-bit page size extension\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_MMX) kprintf ("  MMX instructions\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_SSE) kprintf ("  SSE extensions\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_SSE2) kprintf ("  SSE2 extensions\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_HHT) kprintf ("  Hyper Threading Tech.\n");
  if (cpuidret[2] & CPUID_FEATURE1_EDX_TM) kprintf ("  Therm. Monitor\n");


  uint32_t lo, hi;
  cpuGetMSR (IA32_MSR_TSC, &lo, &hi);
  kprintf ("CPU Clocks Ticks %x %x \n", hi, lo);


  cpuid(2, 0, cpuidret);
  kprintf("CPUID 2: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  
  int i;
  uint8_t* cpu_cache = (uint8_t*)cpuidret;
  for (i = 1; i < 12; ++i) {
    if (cpu_cache[i] == 0) 
      continue;
    kprintf ("%2x]  ", cpu_cache[i]);
    kprintf ("???\n");
  }


  // cpuid(3, 0, cpuidret);
  // kprintf("CPUID 3: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  
  cpuid(4, 0, cpuidret);
  kprintf("CPUID 4: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  int maxProcCore = (cpuidret[0] >> 26) + 1;
  kprintf("MaxProcessorCore: %d \n", maxProcCore);
  // cpuid(5, 0, cpuidret);
  // kprintf("CPUID 5: %x - %x - %x - %x ...\n", cpuidret[0], cpuidret[1], cpuidret[2], cpuidret[3]);
  enableAPIC();
}



// ---------------------------------------------------------------------------
int kCore_Initialize ()
{
  if (screen._mode == 1) {
    screen._ptr = (uint32_t*)(4 * _Mb_);
  }

  // meminit_r(&kSYS.kheap, (void*)0xD0000000, 0x20000000);


  kprintf ("Kernel Smoke Minimal compiled " _DATE_ " from " _OS_FULLNAME_ "\n");

  cpu_info ();
  for (;;);
}


void ksch_pick()
{

}

void ksch_exit(kProcess_t* proc, int status)
{
  
}

void ksch_stop(int state, kCpuRegs_t* regs)
{
  
}


int page_fault (size_t address, int cause)
{

}


// Error driverx

void IRQ14_Enter ()
{

}


void *vbaAddress = NULL;
int vbaWidth;
int vbaHeight;
int vbaDepth;

void VBA_Set (void* address, int width, int height, int depth)
{
  vbaAddress = address;
  vbaWidth = width;
  vbaHeight = height;
  vbaDepth = depth;
}


int term_event (kStream_t* stm, kEvent_t* event)
{

}


void sys_enter (int no, void* stack)
{
}

