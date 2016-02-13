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
 *      Kernel main header.
 */
#pragma once

/* RUNTIME AND C STANDARD ================================================ */

/* Config AxLibC - Get stdlib basic version (no syscalls) */
#ifndef __EX
#  define __EX
#endif

/* Standard includes */
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>


#include <smkos/types.h>
#include <smkos/assert.h>
#include <smkos/atomic.h>
#include <smkos/llist.h>
#include <smkos/bbtree.h>
#include <smkos/spinlock.h>

/* TYPEDEFS ============================================================== */
typedef struct kDriver kDriver_t;
typedef struct kInode kInode_t;
typedef struct kDevice kDevice_t;
typedef struct kMemArea kMemArea_t;
typedef struct kMemSpace kMemSpace_t;
typedef struct kUser kUser_t;
typedef struct kAssembly kAssembly_t;
typedef struct kSection kSection_t;
typedef struct kProcess kProcess_t;
typedef struct kThread kThread_t;
typedef struct kScheduler kScheduler_t;
typedef struct kSession kSession_t;
typedef struct kPage kPage_t;
typedef struct kPipe kPipe_t;
typedef struct kResx kResx_t;
typedef struct kSubSystem kSubSystem_t;
typedef struct kLine kLine_t;
typedef struct kTerm kTerm_t;
typedef struct kWait kWait_t;
typedef enum kWaitReason kWaitReason_e;

/* FUNCTIONS ============================================================= */
/** Display a message on system logs. */
void kprintf(const char *msg, ...);
/** Block kernel execution for safety after an unexpected result. */
void kpanic(const char *msg, ...);
/** Alloc an object on the kernel heap */
void* kalloc(size_t size);
/** Free an object previously allocated on kernel heap */
void kfree(void* ptr);

const char *kpsize (uintmax_t number);

/* GLOBAL STRUCT ========================================================= */

enum kState {
  KST_IDLE = 0,
  KST_KERNSP,
  KST_USERSP,
  KST_HARDW,
  KSTATES
};


struct kCpu {
  enum kState state_;
  int lockCounter_; /**< A counter for the number of spinlock taken */
  int errno_;
  const char *lastError_;
  kThread_t *current_;
  char* spec_;
  clock_t lastClock_;
  clock_t times_[KSTATES];
};

struct kSys {
  llhead_t driverPool_;
  kInode_t *rootIno_;
  kInode_t *devIno_;
  kInode_t *mntIno_;
  kInode_t *sysIno_;
  kInode_t *procIno_;
  llhead_t inodeLru_;
  llhead_t deviceList_;
  llhead_t userList_;
  llhead_t processes_;

  kMemSpace_t *mspace_;
  kScheduler_t *scheduler_;

  atomic_t pageAvailable_;
  atomic_t pageUsed_;
  size_t objMemory_;

  size_t memMax_;
  int pageMax_;
  int pidAutoInc_;
  int ttyAutoInc_;
  int cpuCount_;
  struct kCpu _cpu[8];
};

extern struct kSys kSYS;
#define kCPU kSYS._cpu[kCpuNo]




#define KALLOC(t)  (t*)kalloc(sizeof(t))


#define __unused(v)  ((void)(v))
#define __seterrno(e)    __seterrno_(e, __AT__)
#define __seterrnoN(e,t)  (t*)__seterrnoN_(e, __AT__)


int __seterrno_(int err, const char* at);
void* __seterrnoN_(int err, const char* at);
int __geterrno();
void kernel_state(enum kState state);


#if 0
static inline int __seterrno_(int err, const char* at)
{
  kCPU.errno_ = err;
  kCPU.lastError_ = at;
  return err;
}

static inline void* __seterrnoN_(int err, const char* at)
{
  kCPU.errno_ = err;
  kCPU.lastError_ = at;
  return NULL;
}

static inline int __geterrno()
{
  return kCPU.errno_;
}


static inline void kernel_state(enum kState state)
{
  clock_t now = clock();
  kCPU.times_[kCPU.state_] += now - kCPU.lastClock_;
  kCPU.state_ = state;
  kCPU.lastClock_ = now;
}
#endif

#ifndef __AX_STR_EX
char* strdup(const char*);
int strcmpi (const char* str1, const char* str2);
int vsnprintf(char *str, size_t lg, const char *format, va_list ap);
#else
#define strdup _strdup
#define strcmpi _strcmpi
#endif

unsigned long long strtoull(const char*, char**, int);

int snprintf(char *, size_t, const char *, ...);
void kstacktrace(size_t MaxFrames);
void kdump(void*, int);
void dbg_ticks();


 int bclearbytes (uint8_t *table, int offset, int length);

