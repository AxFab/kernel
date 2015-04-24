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


/* ----------------------------------------------------------------------- */

#define CAP_SYSTEM    0xffff
#define CAP_ADMIN    0xffff


#define CAP_NOBODY 0
#define CAP_EVERYBODY 1




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
/* ----------------------------------------------------------------------- */