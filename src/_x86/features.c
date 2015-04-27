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
 *      Intel x86 CPU features.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/arch.h>
#include <smkos/kstruct/map.h>
 

void PIT_Initialize (uint32_t frequency);
int cpu_features[4];

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define AP_VECTOR (((size_t)x86_ApStart) >> 12)
#define AE_VECTOR (((size_t)x86_ApError ) >> 12)

// EAX, EBX, EDX, ECX
#define x86_FEATURES_FPU    (0 != (cpu_features[2] & (1 << 0)))
#define x86_FEATURES_VME    (0 != (cpu_features[2] & (1 << 1)))
#define x86_FEATURES_PE     (0 != (cpu_features[2] & (1 << 2)))
#define x86_FEATURES_PSE    (0 != (cpu_features[2] & (1 << 3)))
#define x86_FEATURES_TSC    (0 != (cpu_features[2] & (1 << 4)))
#define x86_FEATURES_MSR    (0 != (cpu_features[2] & (1 << 5)))
#define x86_FEATURES_PAE    (0 != (cpu_features[2] & (1 << 6)))
#define x86_FEATURES_MCE    (0 != (cpu_features[2] & (1 << 7)))
#define x86_FEATURES_CX8    (0 != (cpu_features[2] & (1 << 8)))
#define x86_FEATURES_APIC   (0 != (cpu_features[2] & (1 << 9)))
#define x86_FEATURES_SEP    (0 != (cpu_features[2] & (1 << 11)))
#define x86_FEATURES_MMTR   (0 != (cpu_features[2] & (1 << 12)))
#define x86_FEATURES_PSE36  (0 != (cpu_features[2] & (1 << 17)))
#define x86_FEATURES_X2APIC (0 != (cpu_features[3] & (1 << 21)))
#define x86_FEATURES_MMX    (0 != (cpu_features[2] & (1 << 23)))
#define x86_FEATURES_SSE    (0 != (cpu_features[2] & (1 << 25)))
#define x86_FEATURES_SSE2   (0 != (cpu_features[2] & (1 << 26)))
#define x86_FEATURES_HHT    (0 != (cpu_features[2] & (1 << 28)))
#define x86_FEATURES_TM     (0 != (cpu_features[2] & (1 << 29)))

#define cpu_count (*((uint16_t*)0x7f8))

#define APIC        ((uint32_t*)(2 * _Mb_))
#define APIC_PTR(v) (*(APIC + ((v) / 4)))

#define APIC_ID     (*(APIC + (0x20 / 4)))
#define APIC_VERS   (*(APIC + (0x30 / 4)))
#define APIC_TPR    (*(APIC + (0x80 / 4)))
#define APIC_APR    (*(APIC + (0x90 / 4)))
#define APIC_PPR    (*(APIC + (0xA0 / 4)))
#define APIC_EOI    (*(APIC + (0xB0 / 4)))
#define APIC_RRD    (*(APIC + (0xC0 / 4)))
#define APIC_LRD    (*(APIC + (0xD0 / 4)))
#define APIC_DRD    (*(APIC + (0xE0 / 4)))
#define APIC_SVR      APIC_PTR(0xF0)
#define APIC_ESR    (*(APIC + (0x28 * 4)))
#define APIC_ICR_LOW  (*(APIC + (0x30 * 4)))
#define APIC_LVT3   (*(APIC + (0x37 * 4)))

#define APIC_ENABLE 0x800


/* ----------------------------------------------------------------------- */
void x86_InitializeFPU();
void x86_ActiveCache();
void x86_ActiveFPU();
void x86_ApStart();
void x86_ApError();
void cpuid(int eax, int ecx, int *ret);
void x86_IRQ_handler(int no, void (*handler)());


void x86_InitializeFPU ()
{
  if (!x86_FEATURES_FPU) {
    kprintf ("No FPU detected\n");
    return;
  }

  x86_ActiveFPU();
}

void x86_ReadMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void x86_WriteMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

atomic_t volatile __delayTimer;
void __delayIRQ()
{
  // Timer is in microseconds
  atomic_add (&__delayTimer, 1000000 / CLOCK_HZ);
}

void __delayX(int microsecond)
{
  __delayTimer = 0;
  x86_IRQ_handler (0, __delayIRQ);

  while (__delayTimer < microsecond) cpause();
}

int APIC_ON = 0;

/* ----------------------------------------------------------------------- */
int cpu_no()
{
  if (!APIC_ON)
    return 0;

  return (APIC_ID >> 24) & 0xf;
}


/* ----------------------------------------------------------------------- */
void initialize_smp()
{
  page_t apicPage;
  uint32_t eax, ebx;

  // Request CPU features
  // kprintf ("Initializing multi-processing...\n");
  cpuid(1, 0, cpu_features);
  x86_InitializeFPU ();
  x86_ActiveCache();


  // if (x86_FEATURES_FPU) kprintf ("  OnBoard x87 FPU\n");
  // if (x86_FEATURES_VME) kprintf ("  Virtual 8086 mode extensions\n");
  // if (x86_FEATURES_TSC) kprintf ("  Time Stamp Counter\n");
  // if (x86_FEATURES_MSR) kprintf ("  Model-specific registers\n");
  // if (x86_FEATURES_PAE) kprintf ("  Physical Address Extension\n");
  // if (x86_FEATURES_APIC) kprintf ("  Onboard Advanced Programmable Interrupt Controller\n");
  // if (x86_FEATURES_MMTR) kprintf ("  MMTR features\n");
  // if (x86_FEATURES_PSE36) kprintf ("  36-bit page size extension\n");
  // if (x86_FEATURES_MMX) kprintf ("  MMX instructions\n");
  // if (x86_FEATURES_SSE) kprintf ("  SSE extensions\n");
  // if (x86_FEATURES_SSE2) kprintf ("  SSE2 extensions\n");
  // if (x86_FEATURES_HHT) kprintf ("  Hyper Threading Tech.\n");
  // if (x86_FEATURES_TM) kprintf ("  Therm. Monitor\n");

  //
  if (!x86_FEATURES_MSR) {
    kprintf ("No MSR capability\n");
    return;
  } else if (!x86_FEATURES_APIC) {
    kprintf ("No APIC capability\n");
    return;
  } else if (!x86_FEATURES_X2APIC) {
    // kprintf ("No x2APIC implemented\n");
  }


  x86_ReadMSR(IA32_APIC_BASE_MSR, &eax, &ebx);

  if ((eax & (1 << 11)) == 0) {
    kprintf ("Error APIC is disabled\n");
    return;
  } else if ((eax & (1 << 8)) == 0) {
    kprintf ("Error, this is not the BSP\n");
    return;
  }


  // Map APIC at 1 Mb !
  apicPage = eax & 0xfffff000;
  mmu_resolve (2 * _Mb_, apicPage, VMA_KERNEL | VMA_WRITE, false);

  APIC_ON = 1;
  // kprintf ("Initial APIC ID form cpuid[EAX=1] is %d.\n", cpu_features[1] >> 24);
  // kprintf ("Local APIC ID form apic table is %d.\n",  cpu_no ());


  // START APs ------
  memset ((void *)0x700, 0, 0x100); // Initialize 0x100 bytes of data for AP startup

  // kprintf ("Read APs count %d\n", cpu_count);
  // kprintf ("x86_ApStart is at  0x%x [%d-%x]\n", x86_ApStart, AP_VECTOR, AP_VECTOR);
  // kprintf ("x86_ApError is at  0x%x [%d-%x]\n", x86_ApError, AE_VECTOR, AE_VECTOR);

  PIT_Initialize(CLOCK_HZ);
  sti ();

  // Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts
  APIC_SVR = APIC_SVR | APIC_ENABLE;
  APIC_LVT3 = (APIC_LVT3 & (~0xff)) | (AE_VECTOR & 0xff);

  // Broadcast INIT IPI to all APs
  APIC_ICR_LOW = 0x0C4500;
  __delayX(10000); // 10-millisecond delay loop.

  // Load ICR encoding for broadcast SIPI IP (x2)
  APIC_ICR_LOW = 0x000C4600 | AP_VECTOR;
  __delayX(200); // 200-microsecond delay loop.
  APIC_ICR_LOW = 0x000C4600 | AP_VECTOR;
  __delayX(200); // 200-microsecond delay loop.

  // Wait timer interrupt - We should have init all CPUs
  kprintf ("BSP found a count of %d CPUs\n", cpu_count + 1);
  // for (;;);
  cli();
}

void cpu_sched_ticks()
{
  kprintf(".");
  sched_next(kSYS.scheduler_);
}

void cpu_start_scheduler()
{
  x86_IRQ_handler(0, cpu_sched_ticks);
  PIT_Initialize(CLOCK_HZ);
  sti();
}


