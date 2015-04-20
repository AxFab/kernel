#pragma once
#include <smkos/kernel.h>

#define APIC        ((uint32_t*)_Mb_)
#define APIC_ID       (*(APIC + 0x20 / 4))
#define APIC_VERS     (*(APIC + 0x30 / 4))
#define APIC_TPR      (*(APIC + 0x80 / 4))
#define APIC_APR      (*(APIC + 0x90 / 4))
#define APIC_PPR      (*(APIC + 0xA0 / 4))
#define APIC_EOI      (*(APIC + 0xB0 / 4))
#define APIC_RRD      (*(APIC + 0xC0 / 4))
#define APIC_LRD      (*(APIC + 0xD0 / 4))
#define APIC_DRD      (*(APIC + 0xE0 / 4))
#define APIC_SVR      (*(APIC + 0xF0 / 4))
#define APIC_ESR      (*(APIC + 0x28 * 4))
#define APIC_ICR_LOW  (*(APIC + 0x30 * 4))
#define APIC_LVT3     (*(APIC + 0x37 * 4))

#define APIC_ENABLE 0x800

