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
 *      Intel x86 Interruptions.
 */
#include <smkos/kernel.h>
#include <smkos/kapi.h>
#include <smkos/arch.h>
#include <smkos/kstruct/map.h>
#include <smkos/kstruct/task.h>


/* ----------------------------------------------------------------------- */
/** Hardware Exception:
    0x00  Division by zero
    0x01  Debugger
    0x02  NMI
    0x03  Breakpoint
    0x04  Overflow
    0x05  Bounds
    0x06  Invalid Opcode
    0x07  Coprocessor not available
    0x08  Double fault
    0x09  Coprocessor Segment Overrun (386 or earlier only)
    0x0A  Invalid Task State Segment
    0x0B  Segment not present
    0x0C  Stack Fault
    0x0D  General protection fault
    0x0E  Page fault
    0x0F  reserved
    0x10  Math Fault
    0x11  Alignment Check
    0x12  Machine Check
    0x13  SIMD Floating-Point Exception
*/
void sys_ex (int no, int data)
{
  kpanic("Cpu exception '%x' %x\n", no, data);
}


void(*x86_irq_hanlder[16])();


/* ----------------------------------------------------------------------- */
/**Hardware Interrupt Request
    IRQ 0 (0x20) : System Clock
    IRQ 1 : Keyboard
    IRQ 2 : N/A
    IRQ 3 : Serial port (COM2/COM4)
    IRQ 4 : Serial port (COM1/COM3)
    IRQ 5 : LPT2 (sound card)
    IRQ 6 : Floppy drive
    IRQ 7 : Parallel port (LPT1)
    IRQ 8 (0x70) : Real Time Clock (CMOS)
    IRQ 9 : N/A (PCI)
    IRQ 10 : N/A
    IRQ 11 : N/A (USB)
    IRQ 12 : N/A (PS/2)
    IRQ 13 : Math Coprocessor
    IRQ 14 : Primary HDD
    IRQ 15 : Secondary HDD
*/
void sys_irq (int no, size_t *params)
{
  // kprintf ("<I+%x>", (size_t)params);
  if (kCPU.current_ && kCPU.current_->state_ == SCHED_EXEC) {
    kCPU.current_->stackPtr_ = (size_t)params;
    // kprintf ("<I+%x>", (size_t)params);
    assert (kCPU.current_->stackPtr_ < kCPU.current_->kstack_->limit_);
    assert (kCPU.current_->stackPtr_ > kCPU.current_->kstack_->limit_ - 4096 * 2);
  }

  assert(no >= 0 && no < 16);
  if (x86_irq_hanlder[no] == NULL) {
    kprintf("\033[91mIRQ no %d: ignored !?\033[0m\n", no);
    return;
  }

  x86_irq_hanlder[no]();
}


/* ----------------------------------------------------------------------- */
int system_call (int no, size_t p1, size_t p2, size_t p3, size_t p4, size_t p5);


void sys_call(size_t *params)
{
  assert (kCPU.current_ && kCPU.current_->state_ == SCHED_EXEC);
  kCPU.current_->stackPtr_ = (size_t)params;
  // kprintf ("<S+%x>", (size_t)params);
  assert (kCPU.current_->stackPtr_ < kCPU.current_->kstack_->limit_);
  assert (kCPU.current_->stackPtr_ > kCPU.current_->kstack_->limit_ - 4096 * 2);
  int err = system_call(params[11], params[10], params[9], params[8], params[5], params[4]);
  params[11] = err;
  params[9] = __geterrno();
}

void sys_wait_(size_t *params)
{
  assert (kCPU.current_ && kCPU.current_->state_ == SCHED_EXEC);
  // kprintf ("<W+%x>", (size_t)params);
  kCPU.current_->stackPtr_ = (size_t)params;
  assert (kCPU.current_->stackPtr_ < kCPU.current_->kstack_->limit_);
  assert (kCPU.current_->stackPtr_ > kCPU.current_->kstack_->limit_ - 4096 * 2);
  // kprintf ("Put Thread of %d to blocked state\n", kCPU.current_->process_->pid_);
  sched_stop(kSYS.scheduler_, kCPU.current_, SCHED_BLOCKED);
  sched_next(kSYS.scheduler_);
  assert(0);
  /* @todo -- this is not CPU releated. */
}


/* ----------------------------------------------------------------------- */
void x86_IRQ_handler(int no, void (*handler)())
{
  if (no < 0 || no >= 16)
    return;

  x86_irq_hanlder[no] = handler;
}

void general_protection (size_t errcode)
{
  /* kdump(&errcode, 64); */
  kpanic ("General Protection Fault : '0x%08x'\n", errcode);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
