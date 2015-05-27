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
 *      Implementation of a balanced binary tree: AA-tree.
 */
#ifndef SMKOS_FCNTL_H__
#define SMKOS_FCNTL_H__


#define _Bit(d)  (1 << (d))

#define O_RDONLY             0
#define O_WRONLY            01
#define O_RDWR              02
#define O_ACCMODE           03


#define O_CREAT          _Bit(6)
#define O_EXCL           _Bit(7)
#define O_NOCTTY         _Bit(8)
#define O_TRUNC          _Bit(9)
#define O_APPEND         _Bit(10)
#define O_NONBLOCK       _Bit(11)
#define O_NDELAY         O_NONBLOCK
#define O_DSYNC          _Bit(12) /**< Synchronize data. */
#define O_ASYNC          _Bit(13)
#define O_DIRECT         _Bit(14) /**< Direct disk access. */

#define O_LARGEFILE      _Bit(15)
#define O_DIRECTORY      _Bit(16) /**< Must be a directory. */
#define O_NOFOLLOW       _Bit(17) /**< Do not follow links. */
#define O_NOATIME        _Bit(18)
#define O_CLOEXEC        _Bit(19)
#define O_SYNC           (_Bit(20) | O_DSYNC)
#define O_RSYNC          O_SYNC /**< Synchronize read operations. */
#define O_FSYNC          O_SYNC
#define O_PATH           _Bit(21)


#endif /* SMKOS_FCNTL_H__ */
