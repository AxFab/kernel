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
 *      Define macro for kernel limit and safe guards.
 */
#pragma once

#include <smkos/assert.h>
#include <smkos/alimits.h>

#define FNAME_MAX 256
#define FBUFFER_MAX (64*_Mb_) /**< Maximum size of a file buffer (read/write) */
#define STRING_MAX 512
#define PATH_MAX 8192
#define MAX_LOOP 500
#define CLOCK_HZ 100

#define MAX_FD_PER_PROCESS 0xC000000 /**< We need a limit: 200 milions */


