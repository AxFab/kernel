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
  if (pid != 0)
    return ENOSYS;

  /// @todo assert (kCPU.state_ == KST_USERSP);
  assert (kCPU.current_ != NULL);

  sched_stop (kSYS.scheduler_, kCPU.current_, SCHED_ZOMBIE);
  if (kCPU.current_)
    process_exit(kCPU.current_->process_, 0);
  sched_next(kSYS.scheduler_);
  return EAGAIN;
}

int sys_exec(const char *exec, struct SMK_StartInfo *info)
{
  kProcess_t* process;
  kInode_t* pwd = kCPU.current_->process_->session_->workingDir_;
  kInode_t* ino = search_inode(exec, pwd, 0);
  if (ino == NULL) {
    __seterrno(ENOENT);
    return -1;
  }

  if (load_assembly(ino) == NULL){
    __seterrno(ENOEXEC);
    return -1;
  }

   process = create_child_process(ino, kCPU.current_->process_, info);
   if (!process)
     return -1;
   return process->pid_;
}

int sys_write(int fd, void* data, size_t lg, size_t off) 
{
  kResx_t* resx;

  if (lg > FBUFFER_MAX) {
    __seterrno(ENOSYS);
    return -1;
  }

  resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);
  if (resx == NULL)
    return -1;

  if (fd == 1)
    kprintf (data);

  switch (resx->type_) {
    case S_IFBLK:
    case S_IFREG:
      // return fs_block_write();
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      return fs_pipe_write (resx->ino_, data, lg, 0);

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    default:
      assert(resx->type_ == 0);
      __seterrno(EBADF);
      return -1;
  }
}

int sys_read(int fd, void* data, size_t lg, size_t off) 
{
  int ret = 0;
  kResx_t* resx;

  if (lg > FBUFFER_MAX) {
    __seterrno(ENOSYS);
    return -1;
  }

  resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);
  if (resx == NULL)
    return -1;
  
  switch (resx->type_) {
    case S_IFBLK:
    case S_IFREG:
      // return fs_block_write();
      return -1;

    case S_IFCHR:
    case S_IFIFO:
      ret = fs_pipe_read (resx->ino_, data, lg);
      if (ret == 0) {
        return -1;
      }
      return ret;

    case S_IFDIR:
      __seterrno(EISDIR);
      return -1;

    default:
      assert(resx->type_ == 0);
      __seterrno(EBADF);
      return -1;
  }


}

int sys_mmap(int fd, size_t address, size_t length, int flags)
{
  kResx_t* resx;

  resx = process_get_resx (kCPU.current_->process_, fd, CAP_WRITE);
  if (resx == NULL) {
    __seterrno(EBADF);
    return -1;
  }

  // Check MMAP Rights
  // Map on kProcess
  return ENOSYS;
}

