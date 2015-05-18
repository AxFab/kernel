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
 *      Structure and macros for task managment module.
 */
#pragma once

#include <smkos/kernel.h>
#include <smkos/sync.h>
#include <smkos/arch.h>
#include <smkos/kstruct/map.h>


/* ----------------------------------------------------------------------- */
#define SCHED_ZOMBIE 0
#define SCHED_SLEEP 1 /* Can't be interrupted */
#define SCHED_BLOCKED 2 /* Can be interupted */
#define SCHED_READY 3
#define SCHED_EXEC 4
#define SCHED_ABORT 5


/* ----------------------------------------------------------------------- */
struct kScheduler {
  kThread_t        *anchor_;
  struct spinlock lock_;
  struct semaphore  taskSem_;
  atomic_t          totalWeight_;
};


/* ----------------------------------------------------------------------- */
struct kThread {
  int threadId_;
  kProcess_t *process_;
  kThread_t *schNext_;
  struct llnode taskNd_;
  kMemArea_t *kstack_;
  kMemArea_t *ustack_;
  int state_;
  size_t paramValue_;
  size_t paramEntry_;
  time_t start_;
  size_t stackPtr_;
};



/* ----------------------------------------------------------------------- */
struct kProcess {
  int pid_;
  kAssembly_t* assembly_;
  kSession_t* session_;
  time_t start_;
  kMemSpace_t mspace_;
  struct spinlock lock_;
  struct llnode allNd_;
  struct llnode siblingNd_;
  struct llhead children_;
  struct llhead threads_;
  /* kProcess_t *parent_; */
  int runningTask_;
  page_t pageDir_;
  int exitStatus_;

  int pagePrivate_;
  int pageShared_;

  struct bbtree resxTree_;
  int fdCount_;
};


/* ----------------------------------------------------------------------- */
struct kResx
{
  int oflags_;
  int type_;
  kInode_t *ino_;
  struct bbnode fdNd_;
};


/* ----------------------------------------------------------------------- */
enum kWaitReason {
  WT_PIPE_READ = 1,
  WT_PIPE_WRITE = 2,
  WT_HANDLED = 0x80000,
};

struct kWait
{
  struct mutex * mutex_;
  kWaitReason_e reason_;
  // size_t param_;
  kThread_t *thread_;
  struct llnode lnd_;
  struct llhead *list_;
};


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
