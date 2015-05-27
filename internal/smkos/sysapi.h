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
 *      API of system call routines.
 */
#pragma once

#include <smkos/scall.h>
#include <smkos/types.h>

#define SCALL(r,n,...)    r n (__VA_ARGS__); \
                          void n ## _do  (char*, char **); \
                          void n ## _save  (char*, int, __VA_ARGS__, r)


SCALL(int, sys_reboot, int, int);
SCALL(int, sys_exit, int errcode, int pid);
SCALL(int, sys_exec, const char *exec, struct SMK_StartInfo *info);
SCALL(size_t, sys_mmap, int fd, size_t address, size_t length, size_t offset, int flags);
SCALL(int, sys_pinfo, char* buf, int lg, int what);


SCALL(int, sys_start, const char*name, size_t entry, size_t param);
SCALL(int, sys_stop, int);
SCALL(int, sys_wait, int reason, int param, int timeout);

SCALL(int, sys_open, const char *path, int dirFd, int flags, int mode);
SCALL(int, sys_close, int fd);
SCALL(ssize_t, sys_write, int fd, const void *buf, size_t lg, off_t off);
SCALL(ssize_t, sys_read, int fd, void *buf, size_t lg, off_t off);

