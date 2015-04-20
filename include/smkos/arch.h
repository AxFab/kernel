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
 *      Architecture dependant wrapper API.
 */
#pragma once
#include <smkos/kernel.h>

/* === CPU =============================================================== */
struct tm cpu_get_clock();
void cpu_halt();
void cpu_save_task(kThread_t *thread);
void cpu_run_task(kThread_t *thread);
void cpu_start_scheduler();
void initialize_smp();

/* === MMU =============================================================== */
page_t mmu_newdir();
int mmu_resolve (size_t address, page_t page, int access, bool zero);
page_t mmu_newpage();
void mmu_load_env();
void mmu_map_userspace(kMemSpace_t *sp);

void mmu_prolog ();
/** Function to inform paging module that some RAM can be used by the system. */
void mmu_ram (int64_t base, int64_t length);
int mmu_init ();


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
