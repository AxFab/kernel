#include "ata.h"


void atapiDetect (kAtaDrive_t *dr)
{
  uint8_t cl = inb(dr->_pbase + ATA_REG_LBA1);
  uint8_t ch = inb(dr->_pbase + ATA_REG_LBA2);

  if ((cl == 0x14 && ch == 0xEB) || (cl == 0x69 && ch == 0x96)) {
    kprintf ("ATAPI ");
    dr->_type = IDE_ATAPI;

  } else {
    return;
  }

  outb(dr->_pbase + ATA_REG_COMMAND, (uint8_t)ATA_CMD_IDENTIFY_PACKET);
  DELAY;
}

int ATA_Detect (kAtaDrive_t *dr)
{
  int res, k;
  uint8_t ptr[2048];
  // Select Drive:
  outb(dr->_pbase + ATA_REG_HDDEVSEL, (uint8_t)dr->_disc);
  DELAY;
  // Send ATA Identify Command:
  outb(dr->_pbase + ATA_REG_COMMAND, (uint8_t)ATA_CMD_IDENTIFY);
  DELAY;
  // Polling:
  res = inb(dr->_pbase + ATA_REG_STATUS);

  if (res == 0) {
    kprintf ("no disc \n");
    dr->_type = IDE_NODISC;
    return 0;
  }

  for (;;) {
    res = inb(dr->_pbase + ATA_REG_STATUS);

    if ((res & ATA_SR_ERR)) {
      // Probe for ATAPI Devices:
      dr->_type = IDE_NODISC;
      atapiDetect (dr);

      if (dr->_type == IDE_NODISC) {
        kprintf ("unrecognized disc \n");
        return 0;
      }

      break;

    } else if (!(res & ATA_SR_BSY) && (res & ATA_SR_DRQ))
      kprintf ("ATA ");

    dr->_type = IDE_ATA;
    break;
  }

  // (V) Read Identification Space of the Device:
  insl (dr->_pbase + ATA_REG_DATA, ptr, 128);
  // Read Device Parameters:
  dr->_signature = *((uint16_t *)(ptr + ATA_IDENT_DEVICETYPE));
  dr->_capabilities = *((uint16_t *)(ptr + ATA_IDENT_CAPABILITIES));
  dr->_commandsets  = *((uint32_t *)(ptr + ATA_IDENT_COMMANDSETS));
  dr->_size = (dr->_commandsets & (1 << 26)) ?
              *((uint32_t *)(ptr + ATA_IDENT_MAX_LBA_EXT)) :
              *((uint32_t *)(ptr + ATA_IDENT_MAX_LBA));

  // String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
  for (k = 0; k < 40; k += 2) {
    dr->_model[k] = ptr[ATA_IDENT_MODEL + k + 1];
    dr->_model[k + 1] = ptr[ATA_IDENT_MODEL + k];
  }

  dr->_model[40] = 0; // Terminate String.
  kprintf (" Size: %d Kb, %s \n", dr->_size / 2, dr->_model);
  return 1;
}

