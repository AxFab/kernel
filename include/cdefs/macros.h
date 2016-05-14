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
 *      Macro definitions.
 */
#ifndef _CDEFS_MACROS_H
#define _CDEFS_MACROS_H 1

#define _Kb_ (1024L)
#define _Mb_ (1024L*_Kb_)
#define _Gb_ (1024LL*_Mb_)
#define _Tb_ (1024LL*_Gb_)
#define _Pb_ (1024LL*_Tb_)
#define _Eb_ (1024LL*_Pb_)

#define ALIGN_UP(v,a)      (((v)+(a-1))&(~(a-1)))
#define ALIGN_DW(v,a)      ((v)&(~(a-1)))

#define MIN(a,b)    ((a)<=(b)?(a):(b))
#define MAX(a,b)    ((a)>=(b)?(a):(b))
#define POW2(v)   ((v) != 0 && ((v) & ((v)-1)) == 0)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __AT__  __FILE__ ":" TOSTRING(__LINE__)

#define XOR_32_TO_8(v)  (((v) & 0xff) ^ (((v) >> 8) & 0xff) ^ (((v) >> 16) & 0xff) ^ (((v) >> 24) & 0xff))



#endif  /* _CDEFS_MACROS_H */
