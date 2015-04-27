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
 *      Table of syscalls functions.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/sysapi.h>

typedef int(*kScHandler)(size_t p1, size_t p2, size_t p3, size_t p4, size_t p5);
#define SYS_CALL_ENTRY(n,f)  [n] = ((kScHandler)(f))



static kScHandler system_delegate[128] = 
{
  SYS_CALL_ENTRY (SYS_REBOOT, NULL),
  SYS_CALL_ENTRY (SYS_EXIT, sys_exit),
  SYS_CALL_ENTRY (SYS_PAUSE, NULL),
  SYS_CALL_ENTRY (SYS_WRITE, sys_write),
};


/* ----------------------------------------------------------------------- */
int system_call (int no, size_t p1, size_t p2, size_t p3, size_t p4, size_t p5)
{
  int c;
  // kprintf("SYSCALL %d] %8x, %8x, %8x, %8x, %8x\n", no, p1, p2, p3, p4, p5);
  kprintf("[S%x]", no);

  if (no < 0 || no >= 128 || system_delegate[no] == NULL) {
    __seterrno(ENOSYS);
    return -1;
  }

  for (c = 0; c < 0x8000000; ++c);

  return system_delegate[no] (p1, p2, p3, p4, p5);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

