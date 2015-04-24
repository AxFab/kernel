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
 *      Define macro for system call entries.
 */
#pragma once

#ifndef NULL
#  define NULL ((void*)0)
#endif

int syscall(int no, ...);

int __syscall_1(int n, void *a1);
int __syscall_2(int n, void *a1, void *a2);
int __syscall_3(int n, void *a1, void *a2, void *a3);
int __syscall_4(int n, void *a1, void *a2, void *a3, void *a4);
int __syscall_5(int n, void *a1, void *a2, void *a3, void *a4, void *a5);

#define _V  void*
#define SYSCALL1(n,a1)             __syscall_1(n,(_V)(a1))
#define SYSCALL2(n,a1,a2)          __syscall_2(n,(_V)(a1),(_V)(a2))
#define SYSCALL3(n,a1,a2,a3)       __syscall_3(n,(_V)(a1),(_V)(a2),(_V)(a3))
#define SYSCALL4(n,a1,a2,a3,a4)    __syscall_4(n,(_V)(a1),(_V)(a2),(_V)(a3),(_V)(a4))
#define SYSCALL5(n,a1,a2,a3,a4,a5) __syscall_5(n,(_V)(a1),(_V)(a2),(_V)(a3),(_V)(a4),(_V)(a5))


/* ----------------------------------------------------------------------- */
#define SYS_PAUSE  1
#define SYS_REBOOT  0

#define SYS_YIELD 0x08
#define SYS_TIME 0x09
#define SYS_SLEEP 0x0a
#define SYS_ITIMER 0x0b

#define SYS_EXIT    0x10
#define SYS_EXEC    0x11

#define SYS_WAIT 0x14

#define SYS_CLOSE   0x20
#define SYS_OPEN    0x21
#define SYS_READ    0x22
#define SYS_WRITE   0x23


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
