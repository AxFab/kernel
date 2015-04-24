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
 *      List of all inter-module functions.
 */
#pragma once

#include <smkos/kernel.h>
#include <smkos/arch.h>
#include <smkos/stat.h>


/* === ASSEMBLY ========================================================== */
/** Destroy an assembly. */
void destroy_assembly (kAssembly_t *image);
/** Read an image file and create the corresponding assembly.*/
kAssembly_t *load_assembly (kInode_t *ino);


/* === CPU =============================================================== */
struct tm cpu_get_clock();
void cpu_halt();
void cpu_save_task(kThread_t *thread);
void cpu_run_task(kThread_t *thread);
void cpu_start_scheduler();
void initialize_smp();


/* === DEVICE ============================================================ */
int open_fs(kInode_t* ino);
int close_fs(kInode_t* ino);
void display_inodes();
int mount_device(const char* name, kDevice_t* dev, kDriver_t* fs);
void mount_alls ();
void initialize_vfs();
kDriver_t *register_driver(void (*init)(kDriver_t *));
kDevice_t *create_device(const char* name, kInode_t* underlying, SMK_stat_t *stat, void* info);


int fs_block_read(kInode_t *fp, void* buffer, size_t length, size_t offset);
int fs_pipe_read(kInode_t *ino, void* buf, size_t lg);
size_t fs_pipe_write(kInode_t *ino, const void* buf, size_t lg, int flags);
int fs_event(kInode_t *ino, int type, int value);


/* === INODES ============================================================ */
/** Search an inode on the filetree. */
kInode_t *search_inode (const char *path, kInode_t *dir, int flags);
/** Try to add a new inode on the VFS tree. */
kInode_t *register_inode (const char *name, kInode_t *dir, SMK_stat_t *stat, bool unlock);
/** Create a new inode. */
kInode_t *create_inode(const char* name, kInode_t* dir, int mode, size_t lg);
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes);
/** Function to called to grab an inodes */
int inode_open (kInode_t *ino);
/** Function to release an inodes */
int inode_close (kInode_t *ino);
/** Give the inode a symbolic link is refering to. */
kInode_t *follow_symlink(kInode_t *ino, int *links);


/* === MEMORY AREA ======================================================= */
kMemArea_t* area_get(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length);
/** Find the area holding an address */
kMemArea_t *area_find(kMemSpace_t* sp, size_t address);
/** Will allocate a new segment on the address space */
kMemArea_t* area_map(kMemSpace_t* sp, size_t length, int flags);
/** Will allocate a new segment at a fixed address on the address space */
kMemArea_t *area_map_at (kMemSpace_t* sp, size_t address, size_t length, int flags);
int area_attach(kMemArea_t* area, kInode_t* ino, size_t offset);
int area_grow (kMemSpace_t* sp, kMemArea_t *area, size_t extra_size);
kMemArea_t* area_map_ino(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length, int flags);
void area_unmap(kMemSpace_t* sp, kMemArea_t* area);
/** Initialize a new address space structure with a first user-stack */
int area_init(kMemSpace_t* sp, size_t base, size_t length);
int area_assembly (kMemSpace_t *sp, kAssembly_t* assembly);
void scavenge_area(kMemSpace_t* sp);


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


/* === PAGE FAULT ======================================================== */
int page_fault (size_t address, int cause);


/* === SCHEDULER ========================================================= */
/** Thread a signal */
int sched_signal (int raise, size_t data);
/** Insert a new thread on the scheduler */
void sched_insert(kScheduler_t *sched, kThread_t *task);
void sched_remove(kScheduler_t *sched, kThread_t *thread);
/** Change the status of the current executing task and save the current registers */
void sched_stop (kScheduler_t *sched, kThread_t *thread, int state);
void sched_next(kScheduler_t *sched);


/* === TASKS ============================================================= */
kThread_t *create_thread(kProcess_t *process, size_t entry, size_t param);
void destroy_process (kProcess_t *process);
kProcess_t *create_logon_process(kInode_t* ino, kUser_t* user, kInode_t* dir, const char*cmd);
kProcess_t *create_child_process(kInode_t* ino, kProcess_t* parent, const char*cmd);
void thread_abort (kThread_t* thread);
void process_exit(kProcess_t *process, int status);


/* === USERS ============================================================= */
kUser_t *search_user (const char *name, const char *domain);
kUser_t *create_user(const char* username, int capacity);
void destroy_user (kUser_t *user);


/* ----------------------------------------------------------------------- */
void kstacktrace(size_t MaxFrames);
