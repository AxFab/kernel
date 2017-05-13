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
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/arch.h>
#include <klib/pci.h>

#define PCI_IO_CFG_ADDRESS 0xCF8
#define PCI_IO_CFG_DATA 0xCFC


struct PCI_header {
  uint16_t vendor_; // 0
  uint16_t device_; // 2
  uint16_t command_; // 4
  uint16_t status_; // 6
  uint8_t revision_; // 8
  uint8_t prog_IF_; // 9
  uint8_t subclass_; // 10
  uint8_t class_; // 11
  uint8_t cache_line_size_; // 12
  uint8_t latency_timer_; // 13
  uint8_t header_type_; // 14
  uint8_t bist_;  // 15
  uint32_t bar_[0];
};

void PCI_lookfor_driver(PCI_device_t *pci);

static uint16_t PCI_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
  uint32_t address = 0;

  /* Create configuration address */
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)(slot & 0x1f) << 11;
  address |= (uint32_t)(func & 0x7) << 8;
  address |= (uint32_t)(offset & 0xfc);
  address |= 0x80000000;
  outl(PCI_IO_CFG_ADDRESS, address);

  /* Read the data */
  return (inl(PCI_IO_CFG_DATA) >> ((offset & 2) << 3)) & 0xffff;
}

static uint32_t PCI_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
  uint32_t address = 0;

  /* Create configuration address */
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)(slot & 0x1f) << 11;
  address |= (uint32_t)(func & 0x7) << 8;
  address |= (uint32_t)(offset & 0xfc);
  address |= 0x80000000;
  outl(PCI_IO_CFG_ADDRESS, address);

  /* Read the data */
  return inl(PCI_IO_CFG_DATA);
}

static void PCI_config_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
  uint32_t address = 0;

  /* Create configuration address */
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)(slot & 0x1f) << 11;
  address |= (uint32_t)(func & 0x7) << 8;
  address |= (uint32_t)(offset & 0xfc);
  address |= 0x80000000;
  outl(PCI_IO_CFG_ADDRESS, address);

  /* Write data */
  outl(PCI_IO_CFG_DATA, value);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* This section should desapreared once I'll have kernel modules */

static void PCI_vendor_name(struct PCI_device *pci)
{
  switch (pci->vendor_id_) {
    // #include "pci_db_vendors.h"
    case 0x04B3: pci->vendor_ = "IBM"; break;
    case 0x8086: pci->vendor_ = "Intel corporation"; break;
    case 0x1022: pci->vendor_ = "Intel corporation"; break;
    case 0x80ee: pci->vendor_ = "InnoTek"; break;
    case 0x05ac: pci->vendor_ = "Apple Inc."; break;
    case 0x106b: pci->vendor_ = "Apple Inc."; break;;
  }
}

static void PCI_class_name(struct PCI_device *pci)
{
  switch (pci->classes_ >> 8) {
    case 0x0101: pci->class_ = "IDE Controller"; break;
    case 0x0206: pci->class_ = "PICMG 2.14 Multi Computing"; break;
    case 0x0608: pci->class_ = "RACEway Bridge"; break;
    case 0x0E00: pci->class_ = "I20 Architecture"; break;
    default:
      switch (pci->classes_) {
        // #include "pci_db_classes.h"
        case 0x010000: pci->class_ = "SCSI Bus Controller"; break;
        case 0x010200: pci->class_ = "Floppy Disk Controller"; break;
        case 0x010300: pci->class_ = "IPI Bus Controller"; break;
        case 0x010400: pci->class_ = "RAID Controller"; break;
        case 0x010520: pci->class_ = "ATA Controller (Single DMA)"; break;
        case 0x010530: pci->class_ = "ATA Controller (Chained DMA)"; break;
        case 0x020000: pci->class_ = "Ethernet Controller"; break;
        case 0x020100: pci->class_ = "Token Ring Controller"; break;
        case 0x030000: pci->class_ = "VGA-Compatible Controller"; break;
        case 0x040000: pci->class_ = "Video Device"; break;
        case 0x040100: pci->class_ = "Audio Device"; break;
        case 0x060000: pci->class_ = "Host bridge"; break;
        case 0x060100: pci->class_ = "ISA bridge"; break;
        case 0x060200: pci->class_ = "EISA bridge"; break;
        case 0x060300: pci->class_ = "MCA bridge"; break;
        case 0x060400: pci->class_ = "PCI-to-PCI bridge"; break;
        case 0x068000: pci->class_ = "Bridge Device"; break;
        case 0x088000: pci->class_ = "System Peripheral"; break;
        case 0x0C0300: pci->class_ = "USB (Universal Host)"; break;
        case 0x0C0310: pci->class_ = "USB (Open Host)"; break;
        case 0x0C0320: pci->class_ = "USB2 (Intel Enhanced Host)"; break;
        case 0x0C0330: pci->class_ = "USB3 XHCI"; break;
      }
      break;
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void PCI_check_bus(uint8_t bus);

static void PCI_check_func(uint8_t bus, uint8_t slot, uint8_t func)
{
  uint16_t classes;
  uint8_t secondary;

  classes = PCI_config_read16(bus, slot, func, 10);
  if (classes == 0x0604) {
    secondary = PCI_config_read16(bus, slot, func, 24) >> 8;
    PCI_check_bus(secondary);
  }
}

static void PCI_check_device(uint8_t bus, uint8_t slot)
{
  char tmp[16];
  struct PCI_device *pci;
  uint8_t head;
  uint16_t vendor, device;
  uint32_t classes;
  vendor = PCI_config_read16(bus, slot, 0, 0);
  if (vendor == 0xffff) {
    return;
  }

  PCI_check_func(bus, slot, 0);
  device = PCI_config_read16(bus, slot, 0, 2);
  head = PCI_config_read16(bus, slot, 0, 14) & 0xff;
  classes = PCI_config_read32(bus, slot, 0, 8) >> 8;

  pci = (struct PCI_device*)kalloc(sizeof(struct PCI_device));
  pci->bus_ = bus;
  pci->slot_ = slot;
  pci->vendor_id_ = vendor;
  pci->device_id_ = device;
  pci->classes_ = classes;
  pci->dev_name_ = NULL;
  pci->driver_ = NULL;
  pci->irq_ = PCI_config_read16(bus, slot, 0, 60) & 0xFF;
  pci->bar_0_ = PCI_config_read32(bus, slot, 0, 16);

  // kprintf(" /- BAR0: 0x%x, IRQ:%d\n", pci->bar_0_, pci->irq_);
  if (pci->bar_0_ != 0 && (pci->bar_0_ & 1) == 0 && (pci->bar_0_ & 8) == 0) {
    PCI_config_write32(bus, slot, 0, 16, 0xFFFFFFFF);
    pci->bar_0_size_ = (~PCI_config_read32(bus, slot, 0, 16)) + 1;
    // kprintf(" /- BAR0 size: 0x%x\n", pci->bar_0_size_);
    PCI_config_write32(bus, slot, 0, 16, pci->bar_0_);
    if (pci->bar_0_size_ != 0) {
      snprintf(tmp, 16, "PCI.%02x.%02x", bus, slot);
      // pci->mmio_ = kmmap_phys(pci->bar_0_ & ~7, pci->bar_0_size_, tmp);
      // TODO - Map
    }
  } else {
    pci->iobase_ = pci->bar_0_ & 0xFFFFFFFC;
  }

  /* Those 3 methods will be replaced later by driver database,
   * external to the kernel, capable to load new module/drivers. */
  PCI_vendor_name(pci);
  PCI_class_name(pci);
  PCI_lookfor_driver(pci);

  if (pci->dev_name_) {
    kprintf("PCI.%02x.%02x -- %s, %s - \033[32m%s\033[0m\n",
        bus, slot, pci->vendor_, pci->class_, pci->dev_name_);
  } else if (pci->class_ && pci->vendor_) {
    kprintf("PCI.%02x.%02x -- %s, %s - \033[33mUnsupported device id: 0x%04x\033[0m\n",
        bus, slot, pci->vendor_, pci->class_, pci->device_id_);
  } else {
    kprintf("PCI.%02x.%02x -- \033[31mUnknown device: {%04x-%04x} - %06x\033[0m\n",
        bus, slot, pci->vendor_id_, pci->device_id_, pci->classes_);
  }

  if ((head & 0x80) != 0) {
    return;
  }

  for (head = 1; head < 8; head++) {
    if (PCI_config_read16(bus, slot, 0, 0) != 0xffff) {
      PCI_check_func(bus, slot, head);
    }
  }
}

void PCI_check_bus(uint8_t bus)
{
  uint8_t slot;
  for(slot = 0; slot < 32; slot++) {
    PCI_check_device(bus, slot);
  }
}

void PCI_check_all_buses_force(void)
{
  int bus;
  for(bus = 0; bus < 8; bus++) {
    PCI_check_bus(bus);
  }
}

void PCI_check_all_buses(void)
{
  uint8_t head = PCI_config_read16(0, 0, 0, 14) & 0xff;
  if ((head & 0x80) == 0) {
    PCI_check_bus(0);
    return;
  }

  for(head = 0; head < 8; head++) {
    if (PCI_config_read16(0, 0, head, 0) != 0xffff)
      break;
    PCI_check_bus(head);
  }
}


struct llhead drivers_list = INIT_LLHEAD;

void PCI_register_driver(uint32_t class, uint16_t vendor, int (*init) (PCI_device_t *pci))
{
  PCI_driver_t *drv = (PCI_driver_t*)kalloc(sizeof(PCI_driver_t));
  drv->class_ = class;
  drv->vendor_ = vendor;
  drv->init = init;
  ll_append(&drivers_list, &drv->node_);
}


void PCI_lookfor_driver(PCI_device_t *pci)
{
  PCI_driver_t *drv;

  ll_for_each(&drivers_list, drv, PCI_driver_t, node_) {
    if (pci->classes_ == drv->class_ && pci->vendor_id_ == drv->vendor_) {
      if (drv->init(pci) == 0) {
        pci->driver_ = drv;
      }
    }
  }
}

