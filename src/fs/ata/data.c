#include "ata.h"


#define ATA_SELECT_DELAY do {\
      inb(dr->_pbase + ATA_REG_ALTSTATUS);  \
      inb(dr->_pbase + ATA_REG_ALTSTATUS);  \
      inb(dr->_pbase + ATA_REG_ALTSTATUS);  \
      inb(dr->_pbase + ATA_REG_ALTSTATUS);  \
    } while (0)


int kAta_Polling (kAtaDrive_t* dr)
{
  int i;

  // (I) Delay 400 nanosecond for BSY to be set:
  for (i = 0; i < 4; i++)
    inb(dr->_pbase + ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

  // (II) Wait for BSY to be cleared:
  while (inb(dr->_pbase + ATA_REG_STATUS) & ATA_SR_BSY); // Wait for BSY to be zero.

  uint8_t state = inb(dr->_pbase + ATA_REG_STATUS); // Read Status Register.

  // (III) Check For Errors:
  if (state & ATA_SR_ERR)
    return 2; // Error.

  // (IV) Check If Device fault:
  if (state & ATA_SR_DF)
    return 1; // Device Fault.

  // (V) Check DRQ:
  // -------------------------------------------------
  // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
  if ((state & ATA_SR_DRQ) == 0)
    return 3; // DRQ should be set

  return 0;
}

int kAta_Data(int dir, kAtaDrive_t* dr, uint32_t lba,  uint8_t sects, uint8_t* buf)
{
  uint8_t lbaIO[6] = { 0 };
  int mode, cmd, i;
  int head, cyl, sect;

  // Select one from LBA28, LBA48 or CHS;
  if (lba >= 0x10000000) {
    if (!(dr->_capabilities & 0x200)) {
      kprintf ("Wrong LBA, LBA not supported here");
      return __seterrno (ENOSYS);
    }

    mode  = 2;
    lbaIO[0] = (lba & 0x000000FF) >> 0;
    lbaIO[1] = (lba & 0x0000FF00) >> 8;
    lbaIO[2] = (lba & 0x00FF0000) >> 16;
    lbaIO[3] = (lba & 0xFF000000) >> 24;

  } else if (dr->_capabilities & 0x200)  {
    mode = 1;
    lbaIO[0] = (lba & 0x00000FF) >> 0;
    lbaIO[1] = (lba & 0x000FF00) >> 8;
    lbaIO[2] = (lba & 0x0FF0000) >> 16;
    head = (lba & 0xF000000) >> 24;

  } else {
    mode = 0;
    sect      = (lba % 63) + 1;
    cyl       = (lba + 1  - sect) / (16 * 63);
    head      = (lba + 1  - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
    lbaIO[0] = sect;
    lbaIO[1] = (cyl >> 0) & 0xFF;
    lbaIO[2] = (cyl >> 8) & 0xFF;
  }

  // Wait the device
  while (inb(dr->_pbase + ATA_REG_STATUS) & ATA_SR_BSY);

  // Select Drive from the controller;
  if (mode == 0)
    outb(dr->_pbase + ATA_REG_HDDEVSEL, dr->_disc | head); // Drive & CHS.

  else
    outb(dr->_pbase + ATA_REG_HDDEVSEL, 0xE0 | dr->_disc | head); // Drive & LBA

  // Write Parameters;
  if (mode == 2) {
    /*
    outb(channel, ATA_REG_SECCOUNT1,   0);
    outb(channel, ATA_REG_LBA3,   lba_io[3]);
    outb(channel, ATA_REG_LBA4,   lba_io[4]);
    outb(channel, ATA_REG_LBA5,   lba_io[5]);
    */
  }

  outb(dr->_pbase + ATA_REG_SECCOUNT0, sects);
  outb(dr->_pbase + ATA_REG_LBA0, lbaIO[0]);
  outb(dr->_pbase + ATA_REG_LBA1, lbaIO[1]);
  outb(dr->_pbase + ATA_REG_LBA2, lbaIO[2]);

  // DMA dir |= 0x02
  if (mode < 2) {
    if (dir == 0) cmd = ATA_CMD_READ_PIO;

    if (dir == 1) cmd = ATA_CMD_WRITE_PIO;

    if (dir == 2) cmd = ATA_CMD_READ_DMA;

    if (dir == 3) cmd = ATA_CMD_WRITE_DMA;

  } else {
    if (dir == 0) cmd = ATA_CMD_READ_PIO_EXT;

    if (dir == 1) cmd = ATA_CMD_WRITE_PIO_EXT;

    if (dir == 2) cmd = ATA_CMD_READ_DMA_EXT;

    if (dir == 3) cmd = ATA_CMD_WRITE_DMA_EXT;
  }

  outb(dr->_pbase + ATA_REG_COMMAND, cmd);      // Send the Command.

  // Read/Write
  if (dir == 0) {
    for (i = 0; i < sects; i++) {
      if (kAta_Polling(dr)) {
        // Polling, set error and exit if there is.
        return __seterrno(EIO);
      }

      insw (dr->_pbase , buf, 256);
      buf += 512;
    }

  } else if (dir == 1) {
    for (i = 0; i < sects; i++) {
      if (kAta_Polling(dr)) {
        // Polling, set error and exit if there is.
        return __seterrno(EIO);
      }

      outsw (dr->_pbase , buf, 256);
      buf += 512;
    }

    outb(dr->_pbase + ATA_REG_COMMAND, (mode == 2) ?
         ATA_CMD_CACHE_FLUSH_EXT :
         ATA_CMD_CACHE_FLUSH );
    kAta_Polling(dr);

  } else {
    return __seterrno (EIO);
  }

  return __noerror();
}

// ===========================================================================
int IRQ14_LOCK = 0;

void IRQ14_Enter () 
{
  IRQ14_LOCK = 0;
  if (KLOG_FS) kprintf ("IRQ %d\n", 14);
}

// ===========================================================================
void ATA_WaitIRQ (kAtaDrive_t* dr, int irq)
{
  // int k = 0x80000;
  IRQ14_LOCK = 1; 
  
  if (KLOG_FS) kprintf ("IRQ 14 WAIT\n");
  while (IRQ14_LOCK);
  if (IRQ14_LOCK)
    if (KLOG_FS) kprintf ("IRQ 14 ABORT\n");
  else
    if (KLOG_FS)  kprintf ("IRQ 14 NOTICED\n");
  IRQ14_LOCK = 0;
}

void ATA_Grab () {}
void ATA_Release () {}

int kAtapi_Read2 (kAtaDrive_t* dr, uint32_t lba, uint8_t* buf)
{
  int status;
  uint8_t packet[12] = { ATAPI_CMD_READ, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 };
  size_t size;
  if (KLOG_FS) printf ("ATAPI] read <%d> at lba: %x for 1 sector on %x\n", dr->_pbase, lba, buf);

  // Setup SCSI Packet:
  packet[ 2] = (lba >> 24) & 0xFF;
  packet[ 3] = (lba >> 16) & 0xFF;
  packet[ 4] = (lba >> 8) & 0xFF;
  packet[ 5] = (lba >> 0) & 0xFF;

  ATA_Grab();

  outb(dr->_pbase + ATA_REG_HDDEVSEL, dr->_disc & 0x10);
  ATA_SELECT_DELAY;
  outb(dr->_pbase + ATA_REG_FEATURES, 0);
  outb(dr->_pbase + ATA_REG_LBA1, 2048 & 0xff );
  outb(dr->_pbase + ATA_REG_LBA2, (2048 >> 8) & 0xff );
  outb(dr->_pbase + ATA_REG_COMMAND, ATA_CMD_PACKET);
  while ((status = inb (dr->_pbase + ATA_REG_COMMAND)) & 0x80); /* BUSY */
  while (!((status = inb (dr->_pbase + ATA_REG_COMMAND)) & 0x8) && !(status & 0x1));
  if (status & 0x1) {
    ATA_Release();
    return __seterrno (EIO);
  }

  /* Send ATAPI/SCSI command */
  outsw (dr->_pbase, (uint16_t*)packet, 6);
  ATA_WaitIRQ(dr, 14);
  size = (int) inb (dr->_pbase + ATA_REG_LBA2) << 8;
  size |= (int) inb (dr->_pbase + ATA_REG_LBA1);
  assert (size == 2048);

  insw (dr->_pbase, buf, size / 2);
  ATA_WaitIRQ(dr, 14);
  while ((status = inb (dr->_pbase)) & 0x88);

  ATA_Release();
  return __noerror();
}

int kAtapi_Read (kAtaDrive_t* dr, uint32_t lba,  uint8_t sects, uint8_t* buf)
{
  int words = 1024; // Sector Size. ATAPI drives have a sector size of 2048 bytes.
  int i;
  uint8_t packet[12];
  if (KLOG_FS) printf ("ATAPI] read <%d> at lba: %x for %d sectors on %x\n", dr->_pbase, lba, sects, buf);
  asm ("cli");
  outb(dr->_pbase + ATA_REG_CONTROL, 0x0);

  // Setup SCSI Packet:
  packet[ 0] = ATAPI_CMD_READ;
  packet[ 1] = 0x0;
  packet[ 2] = (lba >> 24) & 0xFF;
  packet[ 3] = (lba >> 16) & 0xFF;
  packet[ 4] = (lba >> 8) & 0xFF;
  packet[ 5] = (lba >> 0) & 0xFF;
  packet[ 6] = 0x0;
  packet[ 7] = 0x0;
  packet[ 8] = 0x0;
  packet[ 9] = sects;
  packet[10] = 0x0;
  packet[11] = 0x0;
  // Select the drive:
  outb(dr->_pbase + ATA_REG_HDDEVSEL, dr->_disc & 0x10);

  // Delay 400 nanosecond for BSY to be set:
  for (i = 0; i < 4; i++)
    inb(dr->_pbase + ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

  // Inform the Controller:
  outb(dr->_pbase + ATA_REG_FEATURES, 0);               // Use PIO mode
  outb(dr->_pbase + ATA_REG_LBA1, (words * 2) & 0xFF);  // Lower Byte of Sector Size.
  outb(dr->_pbase + ATA_REG_LBA2, (words * 2) >> 8);    // Upper Byte of Sector Size.
  outb(dr->_pbase + ATA_REG_COMMAND, ATA_CMD_PACKET);   // Send the Command.

  // Waiting for the driver to finish or return an error code
  if (kAta_Polling(dr))
    return __seterrno(EIO);

  // Sending the packet data
  outsw(dr->_pbase, packet, 6);

  // Receiving Data
  for (i = 0; i < sects; i++) {
    // ATA_WaitIRQ (dr, 14); // Wait for an IRQ.
    if (kAta_Polling(dr))
      return __seterrno(EIO);

    if (KLOG_FS) kprintf ("ATAPI] Just get data\n");
    insw(dr->_pbase, buf, words);
    buf += (words * 2);
  }

  // Waiting for an IRQ
  // ATA_WaitIRQ (14);
  // kIrq_Wait(0);

  // Waiting for BSY & DRQ to clear
  while (inb(dr->_pbase + ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ));

  return __noerror();
}

int ATA_Read (kInode_t* ino, void* bucket, size_t count, size_t lba)
{
  kAtaDrive_t* dr = (kAtaDrive_t*)ino->devinfo_;
  size_t i;
  if (KLOG_FS) printf ("ATA] read <%d> at lba: %x for %d sectors on %x\n", dr->_pbase, lba, count, bucket);

  if (bucket == NULL)
    return __seterrno (EINVAL);

  switch (dr->_type) {
    case IDE_NODISC:
      return __seterrno (ENODEV);
    case IDE_ATA:

      if (lba + count > dr->_size)
        return __seterrno (ERANGE);

      return kAta_Data (ATA_READ, dr, lba, count, bucket);
    case IDE_ATAPI:

      for (i = 0; i < count; ++i) {
        if (kAtapi_Read (dr, lba + i, 1, ((uint8_t*)bucket) + (i * 2048)))
        // if (kAtapi_Read2 (dr, lba + i, ((uint8_t*)bucket) + (i * 2048)))
          return __geterrno();
      }

      return __noerror();
    default:
      return __seterrno (ENOSYS);
  }
}


int ATA_Write (kInode_t* ino, void* bucket, size_t count, size_t lba)
{
  kAtaDrive_t* dr = (kAtaDrive_t*)ino->devinfo_;

  if (bucket == NULL)
    return __seterrno (EINVAL);

  switch (dr->_type) {
    case IDE_NODISC:
      return __seterrno (ENODEV);
    case IDE_ATAPI:
      return __seterrno (EROFS);
    case IDE_ATA:

      if (lba + count > dr->_size)
        return __seterrno (ERANGE);

      return kAta_Data (ATA_WRITE, dr, lba, count, bucket);
    default:
      return __seterrno (ENOSYS);
  }
}
