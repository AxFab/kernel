#pragma once

// ===========================================================================
//      Must define virtual sectors limits
// ===========================================================================
#define MMU_KERNEL_BASE    0x00000001 // 1 avoid warning!
#define MMU_KERNEL_LIMIT   0x00100000
#define MMU_USERSP_BASE    0x00800000 // should be 4 Mb UP to 0xE0400000 to get 3.5 Gb
#define MMU_USERSP_LIMIT   0xD0000000
#define MMU_KHEAP_BASE     0xD0000000
#define MMU_KHEAP_LIMIT    0xFFC00000


#define PG_BITMAP_ADD      (0x80000)
#define PG_BITMAP_LG       (0x20000)

#define MMU_ACCESS_WR 3
#define MMU_ACCESS_UR 5


#define MMU_LEVEL1() ((uint32_t *)(0xfffff000))
#define MMU_LEVEL1_KRN() ((uint32_t *)(0x2000))
#define MMU_LEVEL2(s) ((uint32_t *)(0xffc00000 | ((s) << 12)))

#define MMU_PREALLOC_DIR  (0x2000) // Kernel directory
#define MMU_PREALLOC_TBL  (0x3000) // Kernel table
#define MMU_PREALLOC_NEW  (0x4000) // Temporary for new pages directory

#define MMU_PREALLOC_STK  (0x6000) // Kernel init stack / Interrupt stack

#define MMU_KSYS (0x5000)

int area_init(kMemSpace_t* sp, size_t base, size_t length);
