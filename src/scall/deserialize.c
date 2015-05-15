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

char* strtok_r(char* , const char*, char **);

static char* parseStr (char **rent)
{
  char * str = strtok_r(NULL, " (,;)=", rent);
  int lg = strlen(str);
  if (!strcmp(str, "NULL"))
    return NULL;
  if (str[0] == str[lg-1] && str[0] == '"') {
    str[lg-1] = '\0';
    return &str[1];
  }
  return str;
}

 int parseChar (char **rent)
{
  char * str = strtok_r(NULL, " (,;)=", rent);
  int lg = strlen(str);
  if (lg == 3 && str[0] == str[2] && str[0] == '\'')
    return str[1];
  if (lg == 4 && str[0] == str[3] && str[0] == '\'' && str[1] == '\\') {
    switch (str[2]) {
    case 'n': return '\n';
    }
  }
    
  return 0;
}

static int parseInt (char **rent)
{
  char * str = strtok_r(NULL, " (,;)=", rent);
  return strtol(str, NULL, 10);
}

/* ----------------------------------------------------------------------- */
void sys_write_do (char* str, char **rent)
{
  int fd = parseInt(rent);
  char *buf = parseStr(rent);
  int lg = parseInt(rent);
  int sk = parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_write(fd, buf, lg, sk);
  assert (ret == res);
}

void sys_write_save (char* snBuf, int snLg, int fd, const char* buf, int lg, int off, ssize_t bytes)
{
  // snprintf (snBuf, snLg, "sys_write (%d:%s, %x:\"%s\", %d, %d) = %d", fd, NULL, buf, buf, lg, off, bytes);
}

/* ----------------------------------------------------------------------- */
void sys_read_do (char* str, char **rent)
{
  int fd = parseInt(rent);
  char *buf = parseStr(rent);
  int lg = parseInt(rent);
  int sk = parseInt(rent);
  int res = parseInt(rent);
  char *tmp = (char*)kalloc(lg); 
  int ret = sys_read(fd, tmp, lg, sk);
  assert (ret == res);
  if (buf && ret > 0)
    assert(!memcmp(buf, tmp, ret));
  kfree(tmp);
}


/* ----------------------------------------------------------------------- */
void sys_exec_do (char* str, char **rent)
{
  int ret, res;
  struct SMK_StartInfo si;
  char* exe = parseStr(rent);
  si.cmd_ = parseStr(rent);
  si.username_ = parseStr(rent);
  si.input_ = parseInt(rent);
  si.output_ = parseInt(rent);
  si.error_ = parseInt(rent);
  si.workingDir_ = parseInt(rent);
  si.flags_ = parseInt(rent);
  si.mainWindow_ = parseInt(rent);
  res = parseInt(rent);
  ret = sys_exec(exe, &si);
  assert (ret == res);
}


/* ----------------------------------------------------------------------- */
void sys_exit_do (char* str, char **rent)
{
  int status = parseInt(rent);
  int pid = parseInt(rent);
  int ret = sys_exit(status, pid);
  assert(0);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
