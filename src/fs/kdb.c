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
 *      Driver for Keyboard.
 */
#include <smkos/kfs.h>

void x86_IRQ_handler(int no, void (*handler)());


// int KDB_read (kInode_t* ino, void* buf, size_t lg)
// {
//   kPipe_t pipe* ino->pipe_;
//   return fs_pipe_read(pipe, buf, lg);
// }

// int KDB_write (kInode_t* ino, void* buf, size_t lg)
// {
//   kPipe_t pipe* ino->pipe_;
//   return fs_pipe_write(pipe, buf, lg);
// }

kDevice_t *devKeyBoard = NULL;

#define EV_KEYUP 10
#define EV_KEYDW 11


/* ----------------------------------------------------------------------- */
void KDB_irq ()
{
  unsigned char rg;

  do {
    rg = inb(0x64);
  } while ((rg & 0x01) == 0);

  rg = inb(0x60) - 1;
  fs_event(devKeyBoard->ino_, rg > 0x80 ? EV_KEYUP : EV_KEYDW, rg & 0x7F);
}


/* ----------------------------------------------------------------------- */
int KDB_mount (kInode_t *dev, const char *name)
{
  SMK_stat_t stat;
  time_t now = time(NULL);

  if (dev != NULL)
    return ENODEV;

  x86_IRQ_handler(1, KDB_irq);

  memset(&stat, 0, sizeof(stat));
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;
  stat.block_ = 2;
  stat.mode_ = S_IFCHR | 0700;
  stat.major_ = KDB_No;
  stat.minor_ = 0;

  devKeyBoard = create_device("kb0", NULL, &stat, NULL);
  return 0;
}


/* ----------------------------------------------------------------------- */
void KDB(kDriver_t *driver)
{
  driver->major_ = KDB_No;
  driver->name_ = strdup("PS2 keyboard");
  driver->mount = KDB_mount;
}
