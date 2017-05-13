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
 *      Support for PCI device.
 */
#ifndef _KLIB_PCI_H
#define _KLIB_PCI_H 1

#include <smkos/kapi.h>
#include <smkos/arch.h>

#define malloc kalloc
#define free kfree

typedef struct PCI_device PCI_device_t;
typedef struct PCI_driver PCI_driver_t;

struct PCI_device {
  uint8_t bus_;
  uint8_t slot_;
  uint16_t vendor_id_;
  uint16_t device_id_;
  uint32_t classes_;
  uint32_t bar_0_;
  uint32_t bar_0_size_;
  int irq_;
  int iobase_;
  const char *vendor_;
  const char *class_;
  const char *dev_name_;
  PCI_driver_t *driver_;
  void *data_;
  uint32_t mmio_;
};

struct PCI_driver
{
  uint32_t class_;
  uint16_t vendor_;
  struct llnode node_;
  int (*init) (PCI_device_t *pci);
};

/* Look for device on every available bus and slot. */
void PCI_check_all_buses(void);
/* Look for device on every posible bus and slot. */
void PCI_check_all_buses_force(void);

void PCI_register_driver(uint32_t class, uint16_t vendor, int (*init) (PCI_device_t *pci));


static inline void PCI_write_cmd(PCI_device_t *pci, uint16_t address, uint32_t value)
{
  if (pci->bar_0_ & 1) {
    outl(pci->iobase_, address);
    outl((pci->bar_0_ & 0xFFFFFFFC) + 4, value);
  } else {
    *((volatile uint32_t*)(pci->mmio_ + address)) = value;
  }
}

static inline uint32_t PCI_read_cmd(PCI_device_t *pci, uint16_t address)
{
  if (pci->bar_0_ & 1) {
    outl(pci->bar_0_ & 0xFFFFFFFC, address);
    return inl((pci->bar_0_ & 0xFFFFFFFC) + 4);
  } else {
    return *((volatile uint32_t*)(pci->mmio_ + address));
  }
}


typedef int (*IRQ_handler_f)(void*);
static inline int IRQ_register(int no, IRQ_handler_f handler, void *arg)
{
  kprintf ("IRQ register %d\n", no);
  return 0;
}

#endif  /* _KLIB_PCI_H */
