#ifndef SCHEDULER_H__
#define SCHEDULER_H__

#include <kernel/core.h>
#include <kernel/task.h>



// ---------------------------------------------------------------------------
void kevt_wait(kThread_t* task, int event, long param, kCpuRegs_t* regs);
void kevt_cancel (kThread_t* task);
// ---------------------------------------------------------------------------
// int ksch_timeslice (kThread_t* task);
void ksch_ticks (kCpuRegs_t* regs) ;
void ksch_pick ();
// ---------------------------------------------------------------------------
int ksch_create_process (kProcess_t* proc, kInode_t* image, kInode_t* dir, const char* cmd);
int process_login(void* user, kInode_t* prg, kInode_t* dir, kInode_t* tty, const char* cmd);
int ksch_add_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void ksch_destroy_process (kProcess_t* proc);
void ksch_exit (kProcess_t* proc, int status);
// ---------------------------------------------------------------------------
void ksch_init ();
int ksch_ontask ();
void ksch_wakeup (kThread_t* task);
void ksch_stop (int state, kCpuRegs_t* regs);
void ksch_abort (kThread_t* task);
// ---------------------------------------------------------------------------
kThread_t* ksch_new_thread (kProcess_t* proc, uintptr_t entry, intmax_t arg);
void ksch_resurect_thread (kThread_t* task, uintptr_t entry, intmax_t arg) ;
void ksch_destroy_thread (kThread_t* task) ;
// ---------------------------------------------------------------------------

/** The event ticks check if some timers are expired. */
void kevt_ticks();
void task_pause();

#endif /* SCHEDULER_H__ */
