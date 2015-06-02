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
 *      Intel x86 MMU wrapper header.
 */
#pragma once

/* ==========================================================================
      Must define virtual sectors limits
========================================================================== */
#define MMU_KERNEL_BASE    0x00000001 /* 1 avoid warning! */
#define MMU_KERNEL_LIMIT   0x00100000
#define MMU_USERSP_BASE    0x00800000 /* should be 4 Mb UP to 0xE0400000 to get 3.5 Gb */
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
#define MMU_LEVELPG(s) ((uint32_t *)(0xffc00000 | ((s) >> 10)))

#define MMU_PREALLOC_DIR  (0x2000) /* Kernel directory */
#define MMU_PREALLOC_TBL  (0x3000) /* Kernel table */
#define MMU_PREALLOC_NEW  (0x4000) /* Temporary for new pages directory */

#define MMU_PREALLOC_STK  (0x6000) /* Kernel init stack / Interrupt stack */

#define MMU_KSYS (0x5000)

int area_init(kMemSpace_t* sp, size_t base, size_t length);
