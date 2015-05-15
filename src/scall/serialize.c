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


void sys_write_save (char* snBuf, int snLg, int fd, const char* buf, int lg, int off, ssize_t bytes)
{
  const char* ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);
  if (resx != NULL)
    ino = resx->ino_->name_;
  snprintf (snBuf, snLg, "sys_write (%d:%s, %x:\"%s\", %d, %d) = %d", fd, ino, buf, buf, lg, off, bytes);
}

void sys_read_save (char* snBuf, int snLg, int fd, char* buf, int lg, int off, ssize_t bytes)
{
  const char* ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);
  if (resx != NULL)
    ino = resx->ino_->name_;
  buf[bytes] = '\0';
  snprintf (snBuf, snLg, "sys_read (%d:%s, %x:\"%s\", %d, %d) = %d", fd, ino, buf, buf, lg, off, bytes);
}


void  sys_exec_save(char* snBuf, int snLg, const char* exe, SMK_StartInfo_t *si, int ret)
{
  kResx_t *resx;
  const char* ino0 = NULL;
  const char* ino1 = NULL;
  const char* ino2 = NULL;
  const char* ino3 = NULL;
  
  resx = process_get_resx (kCPU.current_->process_, si->input_, 0);
  if (resx != NULL)
    ino0 = resx->ino_->name_;
  
  resx = process_get_resx (kCPU.current_->process_, si->output_, 0);
  if (resx != NULL)
    ino1 = resx->ino_->name_;
  
  resx = process_get_resx (kCPU.current_->process_, si->error_, 0);
  if (resx != NULL)
    ino2 = resx->ino_->name_;
  
  resx = process_get_resx (kCPU.current_->process_, si->workingDir_, 0);
  if (resx != NULL)
    ino3 = resx->ino_->name_;

  snprintf (snBuf, snLg, "sys_exec (%x:\"%s\", {\"%s\", \"%s\", %d:%s, %d:%s, %d:%s, %d:%s, %x, %d}) = %d", exe, exe, 
    si->cmd_, si->username_, si->input_, ino0, si->output_, ino1, si->error_, ino2, si->workingDir_, ino3,
    si->flags_, si->mainWindow_, ret);
}


void sys_exit_save(char* snBuf, int snLg, int status, int ret)
{
  snprintf (snBuf, snLg, "sys_exit (%d, %d)", status, ret);
}

