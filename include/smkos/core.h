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
*      Regroup main headers and methods.
*/
#ifndef _SMKOS_CORE_H
#define _SMKOS_CORE_H 1

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <smkos/atomic.h>
#include <smkos/check.h>


int printf(const char*, ...);
void* malloc(size_t);


#endif  /* _SMKOS_CORE_H */
