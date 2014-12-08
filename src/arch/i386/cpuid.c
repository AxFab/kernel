#include <kernel/core.h>
#include <kernel/mmu.h>


#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800

void cpuid(int leave, int sublv, int* cpu);

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
 
void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
 
/* Set the physical address for local APIC registers */
void cpuSetAPICBase(uintptr_t apic)
{
  uint32_t edx = 0;
  uint32_t eax = (apic & 0xfffff100) | IA32_APIC_BASE_MSR_ENABLE;

#ifdef __PHYSICAL_MEMORY_EXTENSION__
  edx = (apic >> 32) & 0x0f;
#endif

  cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

 
/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpuGetAPICBase()
{
   uint32_t eax, edx;
   cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
 
#ifdef __PHYSICAL_MEMORY_EXTENSION__
   return (eax & 0xfffff100) | ((edx & 0x0f) << 32);
#else
   return (eax & 0xfffff100);
#endif
}

//       ============================== ============================== 
//       ============================== ============================== 

int cpuid_features[4]; // EAX, EBX, EDX, ECX

#define x86_FEATURES_FPU   (0 != (cpuid_features[2] & (1 << 0)))
#define x86_FEATURES_MSR   (0 != (cpuid_features[2] & (1 << 5)))
#define x86_FEATURES_APIC  (0 != (cpuid_features[2] & (1 << 9)))
#define x86_FEATURES_MTRR  (0 != (cpuid_features[2] & (1 << 12)))
#define x86_FEATURES_X2APIC  (0 != (cpuid_features[3] & (1 << 21)))

// int x86_Get4KMemType(size_t base)
// {
//   // if (x86_MTRRCAP_FIX && x86_MMTRCAP_FE) 
//     return -1;
// }

// int x86_MemTypeGet(size_t base, size_t length)
// {
//   if (!x86_FEATURES_MTRR)
//     return -1;

//   base = ALIGN_DW (base, PAGE_SIZE);
//   size = ALIGN_UP (size, PAGE_SIZE);
//   if (base + size < base || base + size > 4 * _Gb_) 
//     return -1;

//   if (x86_MMTR_E == 0)
//     return x86_MMTR_UC;

//   int firstType = x86_Get4KMemType(base);
//   size /= PAGE_SIZE;
//   while (--size > 0) {
//     base += PAGE_SIZE;
//     if (firstType != x86_Get4KMemType(base))
//       return x86_MMTR_MixedTypes;
//   }

//   return firstType;
// }

// ==============================

void x86_GetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  if (x86_FEATURES_MSR) {
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
  }
}
 
void x86_SetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
  if (x86_FEATURES_MSR) {
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
  }
}
 

void x86_ActiveFPU();
void x86_ActiveCache();
void x86_InitializeFPU ()
{
  if (!x86_FEATURES_FPU) {
    kprintf ("No FPU detected\n");
    return;
  }

  x86_ActiveFPU();
}


/*
x86 Initialization
  IDT
  GDT
  TSS
  LDT (optional) 
  MMU (for paging)
  MMTRs
*/

void ap_start ();
void acpi_err ();
void cpu_svr();
 
#define APIC        ((uint32_t*)_Mb_)
#define APIC_ID     (*(APIC + 0x20 / 4))
#define APIC_VERS   (*(APIC + 0x30 / 4))
#define APIC_TPR    (*(APIC + 0x80 / 4))
#define APIC_APR    (*(APIC + 0x90 / 4))
#define APIC_PPR    (*(APIC + 0xA0 / 4))
#define APIC_EOI    (*(APIC + 0xB0 / 4))
#define APIC_RRD    (*(APIC + 0xC0 / 4))
#define APIC_LRD    (*(APIC + 0xD0 / 4))
#define APIC_DRD    (*(APIC + 0xE0 / 4))
#define APIC_SVR    (*(APIC + 0xF0 / 4))
#define APIC_ESR    (*(APIC + 0x28 * 4))
#define APIC_ICR_LOW  (*(APIC + 0x30 * 4))
#define APIC_LVT3   (*(APIC + 0x37 * 4))

#define APIC_ENABLE 0x800

extern int ap_count;

int cpu_id ()
{
  return (APIC_ID >> 24) & 0xf;
}


void __step () 
{
  static int step = 0;
  kprintf ("step %d\n", step);
  ++ step;
}


void enableAPIC()
{
  cpuid(1, 0, cpuid_features);
  x86_InitializeFPU ();
  x86_ActiveCache();

  // MMTR  - clear all to zero ! (uncached memory type)
  // SSE ... SSSE3

  if (!x86_FEATURES_MSR) {
    kprintf ("No MSR capability\n");
    return;
  } else if (!x86_FEATURES_APIC) {
    kprintf ("No APIC capability\n");
    return;
  } else if (!x86_FEATURES_X2APIC)
    kprintf ("No x2APIC implemented\n");


  uint32_t eax, ebx;
  cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &ebx);
  if ((eax & (1 << 11)) == 0) {
    kprintf ("Error APIC is disabled\n");
    return;
  } else if ((eax & (1 << 8)) == 0) {
    kprintf ("Error, this is not the BSP\n");
    return;
  }

  page_t apicPage = eax & 0xfffff000;
  mmu_resolve (_Mb_, apicPage, VMA_KERNEL | VMA_WRITE, false);

  kprintf ("Initial APIC ID form cpuid[EAX=1] is %d.\n", cpuid_features[1] >> 24);
  kprintf ("Local APIC ID form apic table is %d.\n",  cpu_id ());

  if (x86_FEATURES_X2APIC)
    kprintf ("Support of x2APIC: YES\n");
  else
    kprintf ("Support of x2APIC: NO\n");

  kprintf ("---- Local APIC\n");
  kprintf ("    APIC ID    %08x\n", APIC_ID);
  kprintf ("    APIC VERS  %08x\n", APIC_VERS);
  kprintf ("    APIC TPR   %08x\n", APIC_TPR);
  kprintf ("    APIC APR   %08x\n", APIC_APR);
  kprintf ("    APIC PPR   %08x\n", APIC_PPR);
  kprintf ("    APIC EIO   %08x\n", APIC_EOI);
  kprintf ("    APIC RRD   %08x\n", APIC_RRD);
  kprintf ("    APIC LDR   %08x\n", APIC_LRD);
  kprintf ("    APIC DRD   %08x\n", APIC_DRD);
  kprintf ("    APIC SVR   %08x\n", APIC_SVR);
  kprintf ("    APIC Err   %08x\n", APIC_ESR);

  // START APs ------
  int apVector = (uint32_t)ap_start >> 12;
  int aeVector = (uint32_t)acpi_err >> 12;
  
  kprintf ("Read APs count %d\n", ap_count);
  kprintf ("ap_start is at  0x%x [%d-%x]\n", ap_start, apVector, apVector);
  kprintf ("acpi_err is at  0x%x [%d-%x]\n", acpi_err, aeVector, aeVector);

  PIT_Initialize(CLOCK_HZ);
  sti ();
  // for (;;);

  // Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts
  APIC_SVR = APIC_SVR | APIC_ENABLE;
  APIC_LVT3 = (APIC_LVT3 & (~0xff)) | (aeVector & 0xff);

  __step();
  // Broadcast INIT IPI to all APs
  APIC_ICR_LOW = 0x0C4500;
  // __delay(); // 10-millisecond delay loop.
  __delayX(10000); 

  __step();
  // Load ICR encoding for broadcast SIPI IP
  APIC_ICR_LOW = 0x000C4600 | apVector;
  // __delay(); // 200-microsecond delay loop.
  __delayX(200); 

  __step();
  APIC_ICR_LOW = 0x000C4600 | apVector;
  // __delay(); // 200-microsecond delay loop.
  __delayX(200); 

  __step();
  // Wait timer interrupt
  kprintf ("Read APs count %d\n", ap_count);
 


  // cpu_svr();
  // mmu_dump();



    // ---------------

    // int boot_id = cpu_id ();
    // kprintf ("Load APIC_ID %d \n", boot_id);
    // kprintf ("AP startup code is at %x [%x]", ap_start, (size_t)ap_start >> 12);

  

}

extern int PIT_Period;
int volatile __timer = 0;
void ksch_ticks (kCpuRegs_t* regs) 
{
  __timer+= 100; //= PIT_Period;
  // kprintf (".\n");
}


int __delayX (int microsecond) 
{
  sti();
  __timer = 0;
  while (__timer < microsecond);
}

