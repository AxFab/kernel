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
 *      Synchronize tasks and events
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/task.h>


void wait_for(struct mutex * mutex, kWaitReason_e reason, struct llhead* list)
{
  kPipe_t *pipe;
  kWait_t* wait = KALLOC(kWait_t);
  wait->mutex_ = mutex;
  wait->reason_ = reason;
  wait->list_ = list;
  wait->thread_ = kCPU.current_;
  ll_append(list, &wait->lnd_);
  
  mtx_unlock(mutex);
  cpu_wait();
  mtx_lock(mutex);
  ll_remove(list, &wait->lnd_);
  kfree(wait);
}


