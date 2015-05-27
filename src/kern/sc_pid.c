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
 *      Implements system calls: Process Management.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/sysapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>
#include <smkos/file.h>

int sys_exit(int errcode, int pid)
{
  int err;
  kProcess_t *process = kCPU.current_->process_;

  if (pid != 0)
    return ENOSYS;

  /// @todo assert (kCPU.state_ == KST_USERSP);
  assert (kCPU.current_ != NULL);

  err = sched_stop (kSYS.scheduler_, kCPU.current_, SCHED_ZOMBIE);

  if (err == 0)
    process_exit(process, 0);

  sched_next(kSYS.scheduler_);
  return EAGAIN;
}

int sys_exec(const char *exec, struct SMK_StartInfo *info)
{
  kProcess_t *process;
  kInode_t *pwd = kCPU.current_->process_->session_->workingDir_;
  kInode_t *ino = search_inode(exec, pwd, 0);

  if (ino == NULL) {
    __seterrno(ENOENT);
    return -1;
  }

  if (load_assembly(ino) == NULL) {
    __seterrno(ENOEXEC);
    return -1;
  }

  inode_open(ino);
  process = create_child_process(ino, kCPU.current_->process_, info);

  if (!process)
    return -1;

  return process->pid_;
}


/* ----------------------------------------------------------------------- */
int sys_start (const char *name, size_t entry, size_t param)
{
  kThread_t *thread = create_thread(kCPU.current_->process_, entry, param);
  // rename_thread(thread, name);
  return thread != NULL ? 0 : __geterrno();
}


/* ----------------------------------------------------------------------- */
int sys_stop (int param)
{
  __unused(param);
  sched_stop (kSYS.scheduler_, kCPU.current_, SCHED_ZOMBIE);
  sched_next(kSYS.scheduler_);
  return EAGAIN;
}


/* ----------------------------------------------------------------------- */
int sys_wait (int reason, int param, int timeout)
{
  if (reason == 0)
    cpu_wait();

  return -1;
}


/* ----------------------------------------------------------------------- */
size_t sys_mmap(int fd, size_t address, size_t length, size_t offset, int flags)
{
  kResx_t *resx;
  kMemArea_t *area = NULL;
  kMemSpace_t *sp = &kCPU.current_->process_->mspace_;

  // Check MMAP Rights
  // kprintf ("Request mmap with %d, %x, %x, %x, %x\n", fd, address, length, offset, flags);

  if (fd >= 0) {
    resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);
    if (resx == NULL) {
      __seterrno(EBADF);
      return 0;
    }
    __seterrno(ENOSYS);
    return 0;
  } else {
    klock(&sp->lock_);
    area = area_map(sp, length, flags);
    kunlock(&sp->lock_);
  }

  // area_display(sp);
  return (area == NULL) ? 0 :  area->address_;
}



/* ----------------------------------------------------------------------- */
int sys_pinfo (char* buf, int lg, int what)
{
  kInode_t* ino = kCPU.current_->process_->session_->workingDir_;
  inode_readlink(ino, buf, lg);
  __seterrno(0);
  return 0;
}



