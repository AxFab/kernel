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
 *      Implements system calls: Parse strace.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
// #include <smkos/sysapi.h>
#include <smkos/klimits.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>
#include <smkos/kstruct/user.h>
#include <smkos/file.h>

int strtol(const char *, char **, int);


static void parseEscape(char* str) {
  int i, j;
  for (i=0, j=0; str[i]; ++i, ++j) {
    if (str[i] == '\\') {
      if (str[i+1] == 'n')
        str[i++] = '\n';

      if (str[i] != '\\')
        continue;
    }

    str[i] = str[j];
  }

  str[i] = '\0';
}
static char *parseStr (char **rent)
{
  char *str = strtok_r(NULL, " (,;)=", rent);
  int lg = strlen(str);

  if (!strcmp(str, "NULL"))
    return NULL;

  if (str[0] == str[lg - 1] && str[0] == '"') {
    str[lg - 1] = '\0';
    parseEscape(&str[1]);
    return &str[1];
  }

  parseEscape(str);
  return str;
}

int parseChar (char **rent)
{
  char *str = strtok_r(NULL, " (,;)=", rent);
  int lg = strlen(str);

  if (lg == 3 && str[0] == str[2] && str[0] == '\'')
    return str[1];

  if (lg == 4 && str[0] == str[3] && str[0] == '\'' && str[1] == '\\') {
    switch (str[2]) {
    case 'n':
      return '\n';
    }
  }

  return 0;
}

static int parseInt (char **rent)
{
  char *str = strtok_r(NULL, " (,;)=", rent);
  return strtol(str, NULL, 0);
}


void log_sys(const char *sbuf);

/* ----------------------------------------------------------------------- */
void sys_open_save (char *snBuf, int snLg, const char *path, int dirFd, int flags, int mode, int p5, int ret)
{
  const char *dir = NULL;
  const char *ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, dirFd, 0);

  if (resx != NULL)
    dir = resx->ino_->name_;

  resx = process_get_resx (kCPU.current_->process_, ret, 0);
  if (resx != NULL)
    ino = resx->ino_->name_;

  snprintf (snBuf, snLg, "sys_open (%p:\"%s\", %d:%s, %x, 0%o) = %d:%s", path, path, dirFd, dir, flags, mode, ret, ino);
}

void sys_open_do (char *str, char **rent)
{
  char sbuf[128];
  char *path = parseStr(rent);
  int dir = parseInt(rent);
  int flags = parseInt(rent);
  int mode = parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_open(path, dir, flags, mode);
  assert (ret == res);
  sys_open_save(sbuf, 128, path, dir, flags, mode, 0, ret);
  log_sys(sbuf);
}

/* ----------------------------------------------------------------------- */
void sys_close_save (char *snBuf, int snLg, int fd, int p2, int p3, int p4, int p5, int ret)
{
  const char *ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);

  if (resx != NULL)
    ino = resx->ino_->name_;

  snprintf (snBuf, snLg, "sys_close (%d:%s) = %d", fd, ino, ret);
}

void sys_close_do (char *str, char **rent)
{
  char sbuf[128];
  int fd = parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_close(fd);
  assert (ret == res);
  sys_close_save(sbuf, 128, fd, 0, 0, 0, 0, ret);
  log_sys(sbuf);
}

/* ----------------------------------------------------------------------- */
void sys_write_save (char *snBuf, int snLg, int fd, const void *buf, size_t lg, off_t off, int p5, ssize_t bytes)
{
  const char *ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);

  if (resx != NULL)
    ino = resx->ino_->name_;

  snprintf (snBuf, snLg, "sys_write (%d:%s, %p:\"%s\", %d, %d) = %d", fd, ino, buf, (char *)buf, (int)lg, (int)off, (int)bytes);
}

void sys_write_do (char *str, char **rent)
{
  char sbuf[128];
  int fd = parseInt(rent);
  char *buf = parseStr(rent);
  size_t lg = parseInt(rent);
  off_t sk = parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_write(fd, buf, lg, sk);
  assert (ret == res);
  sys_write_save(sbuf, 128, fd, buf, lg, sk, 0, ret);
  log_sys(sbuf);
}


/* ----------------------------------------------------------------------- */
void sys_read_save (char *snBuf, int snLg, int fd, void *buf, size_t lg, off_t off, int p5, ssize_t bytes)
{
  const char *ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);

  if (resx != NULL)
    ino = resx->ino_->name_;

  ((char *)buf)[MIN(bytes, 20)] = '\0';
  snprintf (snBuf, snLg, "sys_read (%d:%s, %p:\"%s\", %d, %d) = %d", fd, ino, buf, (char *)buf, (int)lg, (int)off, (int)bytes);
}

void sys_read_do (char *str, char **rent)
{
  char sbuf[128];
  int fd = parseInt(rent);
  char *buf = parseStr(rent);
  size_t lg = parseInt(rent);
  off_t sk = parseInt(rent);
  int res = parseInt(rent);
  char *tmp = (char *)kalloc(lg + 1);
  int ret = sys_read(fd, tmp, lg, sk);
  assert (ret == res);

  if (buf && ret > 0)
    assert(!memcmp(buf, tmp, ret));

  sys_read_save(sbuf, 128, fd, tmp, lg, sk, 0, ret);
  log_sys(sbuf);
  kfree(tmp);
}

/* ----------------------------------------------------------------------- */
void sys_exec_save(char *snBuf, int snLg, const char *exe, SMK_StartInfo_t *si, int p3, int p4, int p5, int ret)
{
  kResx_t *resx;
  const char *ino0 = NULL;
  const char *ino1 = NULL;
  const char *ino2 = NULL;
  const char *ino3 = NULL;

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

  snprintf (snBuf, snLg, "sys_exec (%p:\"%s\", {\"%s\", \"%s\", %d:%s, %d:%s, %d:%s, %d:%s, %x, %d}) = %d", exe, exe,
            si->cmd_, si->username_, si->input_, ino0, si->output_, ino1, si->error_, ino2, si->workingDir_, ino3,
            si->flags_, si->mainWindow_, ret);
}

void sys_exec_do (char *str, char **rent)
{
  char sbuf[128];
  int ret, res;
  struct SMK_StartInfo si;
  char *exe = parseStr(rent);
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
  sys_exec_save(sbuf, 128, exe, &si, 0, 0, 0, ret);
  log_sys(sbuf);
}

/* ----------------------------------------------------------------------- */
void sys_exit_save(char *snBuf, int snLg, int status, int pid, int p3, int p4, int p5, int ret)
{
  __unused(ret);
  snprintf (snBuf, snLg, "sys_exit (%d, %d)", status, pid);
}

void sys_exit_do (char *str, char **rent)
{
  char sbuf[128];
  int status = parseInt(rent);
  int pid = parseInt(rent);
  sys_exit_save(sbuf, 128, status, pid, 0, 0, 0, 0);
  log_sys(sbuf);
  sys_exit(status, pid);
  assert(0);
}

/* ----------------------------------------------------------------------- */
void sys_start_save (char *snBuf, int snLg, const char *name, size_t entry, size_t param, int p4, int p5, int ret)
{
  snprintf (snBuf, snLg, "sys_start (%p:\"%s\", %x, %u) = %d", name, name, entry, param, ret);
}

void sys_start_do (char *str, char **rent)
{
  char sbuf[128];
  char *name = parseStr(rent);
  size_t entry = (size_t)parseInt(rent);
  size_t param = (size_t)parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_start(name, entry, param);
  assert (ret == res);
  sys_start_save(sbuf, 128, name, entry, param, 0, 0, ret);
  log_sys(sbuf);
}

/* ----------------------------------------------------------------------- */
void sys_stop_save (char *snBuf, int snLg, int param, int p2, int p3, int p4, int p5, int ret)
{
  __unused(param);
  __unused(ret);
  snprintf (snBuf, snLg, "sys_stop ()");
}

void sys_stop_do (char *str, char **rent)
{
  char sbuf[128];
  sys_stop_save(sbuf, 128, 0, 0, 0, 0, 0, 0);
  log_sys(sbuf);
  sys_stop(0);
  assert(0);
}

/* ----------------------------------------------------------------------- */
void sys_wait_save (char *snBuf, int snLg, int reason, int param, int timeout, int p4, int p5, int ret)
{
  snprintf (snBuf, snLg, "sys_wait (%x, %d, %d:<%dsec>) = %d", reason, param, timeout, timeout / 1000000, ret);
}

void sys_wait_do (char *str, char **rent)
{
  char sbuf[128];
  int reason = (size_t)parseInt(rent);
  int param = (size_t)parseInt(rent);
  int timeout = (size_t)parseInt(rent);
  int res = parseInt(rent);
  int ret = sys_wait(reason, param, timeout);
  assert (ret == res);
  sys_wait_save(sbuf, 128, reason, param, timeout, 0, 0, ret);
  log_sys(sbuf);
}


/* ----------------------------------------------------------------------- */
void sys_mmap_save (char *snBuf, int snLg, int fd, size_t address, size_t length, size_t offset, int flags, size_t ret)
{
  const char *ino = NULL;
  kResx_t *resx = process_get_resx (kCPU.current_->process_, fd, 0);

  if (resx != NULL)
    ino = resx->ino_->name_;

  snprintf (snBuf, snLg, "sys_mmap (%d:%s, 0x%x, 0x%x, 0x%x, 0x%x) = 0x%x", fd, ino, address, length, offset, flags, ret);
}

void sys_mmap_do (char *str, char **rent)
{
  char sbuf[128];
  int fd = parseInt(rent);
  size_t address = (size_t)parseInt(rent);
  size_t length = (size_t)parseInt(rent);
  size_t offset = (size_t)parseInt(rent);
  int flags = parseInt(rent);
  char* sres = parseStr(rent);
  size_t res = (size_t)strtol(sres, NULL, 0);
  size_t ret = sys_mmap(fd, address, length, offset, flags);
  if (sres[0] == '+')
    assert (ret != 0);
  else
    assert (ret == res);
  sys_mmap_save(sbuf, 128, fd, address, length, offset, flags, ret);
  log_sys(sbuf);
}


/* ----------------------------------------------------------------------- */
void sys_pinfo_save (char *snBuf, int snLg, char* buf, int lg, int what, int p4, int p5, int ret)
{
  snprintf (snBuf, snLg, "sys_pinfo (%p:\"%s\", %d, %d) = %d", buf, buf, lg, what, ret);
}

void sys_pinfo_do (char *str, char **rent)
{
  char sbuf[128];
  char* buf = parseStr(rent);
  size_t lg = (size_t)parseInt(rent);
  size_t what = (size_t)parseInt(rent);
  int res = parseInt(rent);
  char *tmp = (char *)kalloc(lg + 1);
  int ret = sys_pinfo(tmp, lg, what);
  assert (ret == res);

  if (buf && ret > 0)
    assert(!strcmp(buf, tmp));

  sys_pinfo_save(sbuf, 128, tmp, lg, what, 0, 0, ret);
  log_sys(sbuf);
  kfree(tmp);
}

/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
