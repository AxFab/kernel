#include <smkos/kernel.h>
#include <smkos/arch.h>

uint16_t PCI_config_getw (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
  uint32_t address;
  uint32_t lbus  = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  /* create configuration address */
  address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outl(0xCF8, address);
  /* read in the data */
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  tmp = (uint16_t)((inl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
  return tmp;
}


void PCI_check_device(uint8_t bus, uint8_t device)
{
  int i;
  const char* type = NULL;
  const char* vendorStr = NULL;
  uint16_t buf[0x60];
  uint16_t vendor;
  uint16_t headerType;
  uint32_t deviceType;

  /* try and read the first configuration register. Since there are no */
  /* vendors that == 0xFFFF, it must be a non-existent device. */
  vendor = PCI_config_getw(bus, device, 0, 0);
  if (vendor == 0xFFFFU)
    return;

  headerType = PCI_config_getw(bus, device, 0, 0xe) >> 8;

  for (i=0; i<0x20; i++)
    buf[i] = PCI_config_getw(bus, device, 0, i*2);
  deviceType = *((uint32_t*)(&buf[4]));

  switch (deviceType >> 8) {
    case 0x10000: type = "SCSI Bus Controller"; break;
    case 0x10200: type = "Floppy Disk Controller"; break;
    case 0x10300: type = "IPI Bus Controller"; break;
    case 0x10400: type = "RAID Controller"; break;
    case 0x10520: type = "ATA Controller (Single DMA)"; break;
    case 0x10530: type = "ATA Controller (Chained DMA)"; break;
    case 0x20000: type = "Ethernet Controller"; break;
    case 0x20100: type = "Token Ring Controller"; break;
    case 0x30000: type = "VGA-Compatible Controller"; break;
    case 0x40000: type = "Video Device"; break;
    case 0x40100: type = "Audio Device"; break;
    case 0x60000: type = "Host bridge"; break;
    case 0x60100: type = "ISA bridge"; break;
    case 0x60200: type = "EISA bridge"; break;
    case 0x60300: type = "MCA bridge"; break;
    case 0x60400: type = "PCI-to-PCI bridge"; break;
    case 0x68000: type = "Bridge Device"; break;
    case 0x88000: type = "System Peripheral"; break;
    case 0xc0300: type = "USB Controller"; break; // Universal Host Controller Spec
    case 0xc0310: type = "USB Controller"; break; // Open Host Controller Spec
    case 0xc0320: type = "USB Controller"; break; // Intel Enhanced Host Controller Interface
    case 0xc0380: type = "USB Controller"; break;
  }

  switch (vendor) {
    case 0x8086: vendorStr = "Intel corporation"; break;
    case 0x80ee: vendorStr = "InnoTek"; break;
    case 0x1022: vendorStr = "Intel corporation"; break;
    case 0x106b: vendorStr = "Apple Inc."; break;
  }

  kprintf ("%02x:%02x.0 %s: %s \n", bus, device, type, vendorStr, vendor);
  // kdump (buf, 0x40);
}


void PCI_check_bus(uint8_t bus)
{
  int device;
  for (device = 0; device < 32; device++) {
    PCI_check_device(bus, device);
  }
}

void PCI_check_all(void)
{
  int bus;
  uint16_t headerType = PCI_config_getw(0, 0, 0, 0xe);
  kprintf ("PCI detection\n");
  if( (headerType & 0x80) == 0) {
    /* Single PCI host controller */
    PCI_check_bus(0);
  } else {
    /* Multiple PCI host controllers */
    for(bus = 0; bus < 8; bus++) {
      if(PCI_config_getw(0, 0, bus, 0) != 0xFFFF)
        break;
      PCI_check_bus(bus);
    }
  }
}

