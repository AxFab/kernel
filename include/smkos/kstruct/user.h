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


/* ----------------------------------------------------------------------- */

#define CAP_SYSTEM    0xffff
#define CAP_ADMIN    0xffff


#define CAP_NOBODY        0
#define CAP_EVERYBODY     (1 << 0)

#define CAP_READ          (1 << 1)
#define CAP_WRITE         (1 << 2)
#define CAP_EXEC          (1 << 3)



/* ----------------------------------------------------------------------- */
struct kSession {
  kUser_t *user_;
  kInode_t *workingDir_;
  atomic_t usage_;
};


/* ----------------------------------------------------------------------- */
struct kUser
{
  const char* name_;
  int capacity_;
  int processCount_;
  struct llnode allNd_;
};


/* ----------------------------------------------------------------------- */
struct kSubSystem {
  struct mutex mutex_;
  void (*event)(int type, int value);
  int (*write)(const void *m, int lg);
  kTerm_t* term_;
  kInode_t* out_;
  kInode_t* in_;
};


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
