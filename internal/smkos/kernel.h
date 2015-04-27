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

// #include <smkos/_spec.h>

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


/* FUNCTIONS ============================================================= */
/** Display a message on system logs. */
void kprintf(const char *msg, ...);
/** Block kernel execution for safety after an unexpected result. */
void kpanic(const char *msg, ...);
/** Alloc an object on the kernel heap */
void* kalloc(size_t size);
/** Free an object previously allocated on kernel heap */
void kfree(void* ptr);

// void* kmap_ino(kInode_t* ino, size_t offset, size_t length);
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
  kThread_t *current_;
  clock_t lastClock_;
  clock_t times_[KSTATES];
};

struct kSys {
  struct llhead driverPool_;
  kInode_t *rootIno_;
  kInode_t *devIno_;
  kInode_t *mntIno_;
  kInode_t *sysIno_;
  kInode_t *procIno_;
  struct llhead inodeLru_;
  struct llhead deviceList_;
  struct llhead userList_;
  struct llhead processes_;

  kMemSpace_t *mspace_;
  kScheduler_t *scheduler_;
  
  atomic_t pageAvailable_;
  atomic_t pageUsed_;
  size_t objMemory_;

  size_t memMax_;
  int pageMax_;
  int pidAutoInc_;

  struct kCpu _cpu[8];
};

extern struct kSys kSYS;
#define kCPU kSYS._cpu[kCpuNo]




#define KALLOC(t)  (t*)kalloc(sizeof(t))

static inline int __seterrno(int err)
{
  kCPU.errno_ = err;
  return err;
}

#define __unused(v)  ((void)(v))
#define __seterrnoN(e,t)  (t*)__seterrnoN_(e)

static inline void* __seterrnoN_(int err)
{
  kCPU.errno_ = err;
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

char* strdup(const char*);
int strcmpi (const char* str1, const char* str2);
unsigned long long strtoull(const char*, char**, int);
#define exit_() assert(0)
int snprintf(char *, size_t, const char *, ...);
int vsnprintf(char *str, size_t lg, const char *format, va_list ap);

void kstacktrace(size_t MaxFrames);
void kdump(void*, int);

