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
 *      Device support and driver managment.
 */
#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/arch.h>

#define PCI_IO_CFG_ADDRESS 0xCF8
#define PCI_IO_CFG_DATA 0xCFC


const char *class_names[] = {
  "Device was built prior definition of the class code field",
  "Mass Storage Controller",
  "Network Controller",
  "Display Controller",
  "Multimedia Controller",
  "Memory Controller",
  "Bridge Device",
  "Simple Communication Controllers",
  "Base System Peripherals",
  "Input Devices",
  "Docking Stations",
  "Processors",
  "Serial Bus Controllers",
  "Wireless Controllers",
  "Intelligent I/O Controllers",
  "Satellite Communication Controllers",
  "Encryption/Decryption Controllers",
  "Data Acquisition and Signal Processing Controllers",
};

struct PCI_device {
  uint8_t bus_;
  uint8_t slot_;
  uint16_t vendor_id_;
  uint16_t device_id_;
  uint32_t classes_;
  const char *vendor_;
  const char *class_;
};

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

static uint16_t PCI_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
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

void PCI_check_bus(uint8_t bus);

static void PCI_check_func(uint8_t bus, uint8_t slot, uint8_t func)
{
  uint16_t classes;
  uint8_t secondary;

  classes = PCI_config_read(bus, slot, func, 10);
  if (classes == 0x0604 || classes == 0x0406) {
    secondary = PCI_config_read(bus, slot, func, 24) >> 8;
    PCI_check_bus(secondary);
  }
}

static void PCI_vendor_name(struct PCI_device *pci)
{
  switch (pci->vendor_id_) {
    case 0x8086: pci->vendor_ = "Intel corporation"; break;
    case 0x80ee: pci->vendor_ = "InnoTek"; break;
    case 0x1022: pci->vendor_ = "Intel corporation"; break;
    case 0x106b: pci->vendor_ = "Apple Inc."; break;
  }
}

static void PCI_class_name(struct PCI_device *pci)
{
  switch (pci->classes_) {
    case 0x10000: pci->class_ = "SCSI Bus Controller"; break;
    case 0x10200: pci->class_ = "Floppy Disk Controller"; break;
    case 0x10300: pci->class_ = "IPI Bus Controller"; break;
    case 0x10400: pci->class_ = "RAID Controller"; break;
    case 0x10520: pci->class_ = "ATA Controller (Single DMA)"; break;
    case 0x10530: pci->class_ = "ATA Controller (Chained DMA)"; break;
    case 0x20000: pci->class_ = "Ethernet Controller"; break;
    case 0x20100: pci->class_ = "Token Ring Controller"; break;
    case 0x30000: pci->class_ = "VGA-Compatible Controller"; break;
    case 0x40000: pci->class_ = "Video Device"; break;
    case 0x40100: pci->class_ = "Audio Device"; break;
    case 0x60000: pci->class_ = "Host bridge"; break;
    case 0x60100: pci->class_ = "ISA bridge"; break;
    case 0x60200: pci->class_ = "EISA bridge"; break;
    case 0x60300: pci->class_ = "MCA bridge"; break;
    case 0x60400: pci->class_ = "PCI-to-PCI bridge"; break;
    case 0x68000: pci->class_ = "Bridge Device"; break;
    case 0x88000: pci->class_ = "System Peripheral"; break;
    case 0xc0300: pci->class_ = "USB Controller"; break; // Universal Host Controller Spec
    case 0xc0310: pci->class_ = "USB Controller"; break; // Open Host Controller Spec
    case 0xc0320: pci->class_ = "USB Controller"; break; // Intel Enhanced Host Controller Interface
    case 0xc0380: pci->class_ = "USB Controller"; break;
  }
}

static void PCI_check_device(uint8_t bus, uint8_t slot)
{
  struct PCI_device *pci;
  uint8_t head, prog;
  uint16_t vendor, device;
  uint32_t classes;
  vendor = PCI_config_read(bus, slot, 0, 0);
  if (vendor == 0xffff) {
    return;
  }

  PCI_check_func(bus, slot, 0);
  device = PCI_config_read(bus, slot, 0, 2);
  head = PCI_config_read(bus, slot, 0, 14) & 0xff;
  classes = PCI_config_read(bus, slot, 0, 10) << 8| PCI_config_read(bus, slot, 0, 8) >> 8;

  pci = (struct PCI_device*)kalloc(sizeof(struct PCI_device));
  pci->bus_ = bus;
  pci->slot_ = slot;
  pci->vendor_id_ = vendor;
  pci->device_id_ = device;
  pci->classes_ = classes;

  PCI_vendor_name(pci);
  PCI_class_name(pci);

  kprintf("PCI -- <%2x, %2x> %s - %s {%04x-%04x} - %06x\n",
    bus, slot,
    pci->vendor_, pci->class_,
    pci->vendor_id_, pci->device_id_,
    pci->classes_);
  if ((head & 0x80) != 0) {
    return;
  }

  for (head = 1; head < 8; head++) {
    if (PCI_config_read(bus, slot, 0, 0) != 0xffff) {
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
  uint8_t head = PCI_config_read(0, 0, 0, 14) & 0xff;
  if ((head & 0x80) == 0) {
    PCI_check_bus(0);
    return;
  }

  for(head = 0; head < 8; head++) {
    if (PCI_config_read(0, 0, head, 0) != 0xffff)
      break;
    PCI_check_bus(head);
  }
}

