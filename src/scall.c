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
 *      Open and Load assembly file.
 */
#include <smkos/kernel.h>
// #include <smkos/core.h>
#include <smkos/kapi.h>
#include <smkos/sysapi.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>

typedef int(*kScHandler)(size_t p1, size_t p2, size_t p3, size_t p4, size_t p5);
#define SYS_CALL_ENTRY(n,f)  [n] = ((kScHandler)(f))


int sys_write(int fd, void* data, size_t lg, size_t off) 
{
  if (fd == 1)
    kprintf (data);
  return strlen(data);
}


int sys_exit(int errcode, int pid)
{
  if (pid != 0)
    return ENOSYS;

  // todo assert (kCPU.state_ == KST_USERSP);
  assert (kCPU.current_ != NULL);

  sched_stop (kSYS.scheduler_, kCPU.current_, SCHED_ZOMBIE);
  if (kCPU.current_)
    process_exit(kCPU.current_->process_, 0);
  sched_next(kSYS.scheduler_);
  return EAGAIN;
}

int sys_mmap(int fd, size_t address, size_t length, int flags)
{
  // GET Inode of FD
  // Check MMAP Rights
  // Map on kProcess
  return ENOSYS;
}



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
  kprintf("SYSCALL %d] %8x, %8x, %8x, %8x, %8x\n", no, p1, p2, p3, p4, p5);

  if (no < 0 || no >= 128 || system_delegate[no] == NULL) {
    __seterrno(ENOSYS);
    return -1;
  }

  for (c = 0; c < 0x800000; ++c);

  return system_delegate[no] (p1, p2, p3, p4, p5);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */

