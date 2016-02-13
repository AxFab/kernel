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
 */
#ifndef _SKC_STDDEF_H
#define _SKC_STDDEF_H 1

#include <stddef.h>


#ifndef pointerof
# undef offsetof
/** Offset of member MEMBER in a structure of type TYPE. */
# define offsetof(TYPE, MEMBER)  ((size_t)&(((TYPE*)0)->MEMBER))
/** Find the pointer of a structure of type TYPE knowing the ADDRESS of its
 member MEMBER.  */
# define pointerof(ADDRESS, TYPE, MEMBER) \
    ((TYPE*)(!ADDRESS?0:((char*)ADDRESS-offsetof(TYPE,MEMBER))))
#endif


#endif  /* _SKC_STDDEF_H */
