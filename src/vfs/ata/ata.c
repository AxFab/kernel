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
 *      Driver for ATA API.
 */
#include <smkos/kfs.h>


#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01


#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC


#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_NODISC     0x00
#define IDE_ATA        0x01
#define IDE_ATAPI      0x02

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01


#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01
#define      ATA_DMA       0x02



#define ATA_DELAY do {\
      inb(dr->pbase_ + ATA_REG_ALTSTATUS);  \
      inb(dr->pbase_ + ATA_REG_ALTSTATUS);  \
      inb(dr->pbase_ + ATA_REG_ALTSTATUS);  \
      inb(dr->pbase_ + ATA_REG_ALTSTATUS);  \
    } while (0)


struct ATA_Drive {
  uint8_t   type_; // ATA
  uint16_t  pbase_; // 0x1f0 - 0x170
  uint16_t  pctrl_; // 0x3f6 - 0x376
  uint8_t   disc_;  // 0xa0 - 0xb0
  uint16_t signature_;
  uint16_t capabilities_;
  uint32_t commandsets_;
  uint32_t size_;
  char     model_[44];
};

struct ATA_Drive sdx[4];


int IRQ14_LOCK = 0;

/* ----------------------------------------------------------------------- */
void IRQ14_Enter ()
{
  IRQ14_LOCK = 0;
  kprintf ("IRQ %d\n", 14);
}


/* ----------------------------------------------------------------------- */
void ATA_WaitIRQ (struct ATA_Drive *dr, int irq)
{
  // int k = 0x80000;
  IRQ14_LOCK = 1;

  kprintf ("IRQ 14 WAIT\n");

  while (IRQ14_LOCK);

  if (IRQ14_LOCK) {
    kprintf ("IRQ 14 ABORT\n");
  } else {
    kprintf ("IRQ 14 NOTICED\n");
  }

  IRQ14_LOCK = 0;
}


/* ----------------------------------------------------------------------- */
static int ATA_Polling (struct ATA_Drive *dr)
{
  // (I) Delay 400 nanosecond for BSY to be set:
  ATA_DELAY;

  // (II) Wait for BSY to be cleared:
  while (inb(dr->pbase_ + ATA_REG_STATUS) & ATA_SR_BSY); // Wait for BSY to be zero.

  uint8_t state = inb(dr->pbase_ + ATA_REG_STATUS); // Read Status Register.

  // (III) Check For Errors:
  if (state & ATA_SR_ERR) {
    kprintf(" - ATA] device on error\n");
    return 2; // Error.
  }

  // (IV) Check If Device fault:
  if (state & ATA_SR_DF) {
    kprintf(" - ATA] device fault\n");
    return 1; // Device Fault.
  }

  // (V) Check DRQ:
  // -------------------------------------------------
  // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
  if ((state & ATA_SR_DRQ) == 0) {
    kprintf(" - ATA] DRQ should be set\n");
    return 3; // DRQ should be set
  }

  return 0;
}


/* ----------------------------------------------------------------------- */
static int ATA_Data(int dir, struct ATA_Drive *dr, uint32_t lba,  uint8_t sects, uint8_t *buf)
{
  uint8_t lbaIO[6] = { 0 };
  int mode, cmd, i;
  int head, cyl, sect;

  // Select one from LBA28, LBA48 or CHS;
  if (lba >= 0x10000000) {
    if (!(dr->capabilities_ & 0x200)) {
      // kprintf ("Wrong LBA, LBA not supported here");
      return ENOSYS;
    }

    mode  = 2;
    lbaIO[0] = (lba & 0x000000FF) >> 0;
    lbaIO[1] = (lba & 0x0000FF00) >> 8;
    lbaIO[2] = (lba & 0x00FF0000) >> 16;
    lbaIO[3] = (lba & 0xFF000000) >> 24;

  } else if (dr->capabilities_ & 0x200)  {
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
  while (inb(dr->pbase_ + ATA_REG_STATUS) & ATA_SR_BSY);

  // Select Drive from the controller;
  if (mode == 0)
    outb(dr->pbase_ + ATA_REG_HDDEVSEL, dr->disc_ | head); // Drive & CHS.

  else
    outb(dr->pbase_ + ATA_REG_HDDEVSEL, 0xE0 | dr->disc_ | head); // Drive & LBA

  // Write Parameters;
  if (mode == 2) {
    /*
    outb(channel, ATA_REG_SECCOUNT1,   0);
    outb(channel, ATA_REG_LBA3,   lba_io[3]);
    outb(channel, ATA_REG_LBA4,   lba_io[4]);
    outb(channel, ATA_REG_LBA5,   lba_io[5]);
    */
  }

  outb(dr->pbase_ + ATA_REG_SECCOUNT0, sects);
  outb(dr->pbase_ + ATA_REG_LBA0, lbaIO[0]);
  outb(dr->pbase_ + ATA_REG_LBA1, lbaIO[1]);
  outb(dr->pbase_ + ATA_REG_LBA2, lbaIO[2]);

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

  outb(dr->pbase_ + ATA_REG_COMMAND, cmd);      // Send the Command.

  // Read/Write
  if (dir == 0) {
    for (i = 0; i < sects; i++) {
      if (ATA_Polling(dr)) {
        // Polling, set error and exit if there is.
        return EIO;
      }

      insw (dr->pbase_ , buf, 256);
      buf += 512;
    }

  } else if (dir == 1) {
    for (i = 0; i < sects; i++) {
      if (ATA_Polling(dr)) {
        // Polling, set error and exit if there is.
        return EIO;
      }

      outsw (dr->pbase_ , buf, 256);
      buf += 512;
    }

    outb(dr->pbase_ + ATA_REG_COMMAND, (mode == 2) ?
         ATA_CMD_CACHE_FLUSH_EXT :
         ATA_CMD_CACHE_FLUSH );
    ATA_Polling(dr);

  } else {
    return EIO;
  }

  return 0;
}


/* ----------------------------------------------------------------------- */
/*
static int ATAPI_Read2 (struct ATA_Drive *dr, uint32_t lba, uint8_t *buf)
{
  int status;
  uint8_t packet[12] = { ATAPI_CMD_READ, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 };
  size_t size;

  kprintf (" - ATAPI] read %x for 1 sectors on %x\n", lba, buf);
  // kprintf ("ATAPI] read <%d> at lba: %x for 1 sector on %x\n", dr->pbase_, lba, buf);

  // Setup SCSI Packet:
  packet[ 2] = (lba >> 24) & 0xFF;
  packet[ 3] = (lba >> 16) & 0xFF;
  packet[ 4] = (lba >> 8) & 0xFF;
  packet[ 5] = (lba >> 0) & 0xFF;

  ATA_Grab();

  outb(dr->pbase_ + ATA_REG_HDDEVSEL, dr->disc_ & 0x10);
  ATA_DELAY;
  outb(dr->pbase_ + ATA_REG_FEATURES, 0);
  outb(dr->pbase_ + ATA_REG_LBA1, 2048 & 0xff );
  outb(dr->pbase_ + ATA_REG_LBA2, (2048 >> 8) & 0xff );
  outb(dr->pbase_ + ATA_REG_COMMAND, ATA_CMD_PACKET);

  while ((status = inb (dr->pbase_ + ATA_REG_COMMAND)) & 0x80); / * BUSY * /

  while (!((status = inb (dr->pbase_ + ATA_REG_COMMAND)) & 0x8) && !(status & 0x1));

  if (status & 0x1) {
    ATA_Release();
    return EIO;
  }

  / * Send ATAPI/SCSI command * /
  outsw (dr->pbase_, (uint16_t *)packet, 6);
  // ATA_WaitIRQ(dr, 14);
  size = (int) inb (dr->pbase_ + ATA_REG_LBA2) << 8;
  size |= (int) inb (dr->pbase_ + ATA_REG_LBA1);
  assert (size == 2048);

  insw (dr->pbase_, buf, size / 2);
  // ATA_WaitIRQ(dr, 14);

  while ((status = inb (dr->pbase_)) & 0x88);

  ATA_Release();
  return 0;
}
*/

/* ----------------------------------------------------------------------- */
int ATAPI_Read (struct ATA_Drive *dr, uint32_t lba,  uint8_t sects, uint8_t *buf)
{
  int words = 1024; // Sector Size. ATAPI drives have a sector size of 2048 bytes.
  int i;
  uint8_t packet[12];

  // kprintf (" - ATAPI] read %d sector at LBA: %d on %x\n", sects, lba, buf);

  cli();
  outb(dr->pbase_ + ATA_REG_CONTROL, 0x0);

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
  outb(dr->pbase_ + ATA_REG_HDDEVSEL, dr->disc_ & 0x10);

  // Delay 400 nanosecond for BSY to be set:
  ATA_DELAY;

  // Inform the Controller:
  outb(dr->pbase_ + ATA_REG_FEATURES, 0);               // Use PIO mode
  outb(dr->pbase_ + ATA_REG_LBA1, (words * 2) & 0xFF);  // Lower Byte of Sector Size.
  outb(dr->pbase_ + ATA_REG_LBA2, (words * 2) >> 8);    // Upper Byte of Sector Size.
  outb(dr->pbase_ + ATA_REG_COMMAND, ATA_CMD_PACKET);   // Send the Command.

  // Waiting for the driver to finish or return an error code
  if (ATA_Polling(dr))
    return EIO;

  // Sending the packet data
  outsw(dr->pbase_, packet, 6);

  // Receiving Data
  for (i = 0; i < sects; i++) {
    // ATA_WaitIRQ (dr, 14); // Wait for an IRQ.
    if (ATA_Polling(dr))
      return EIO;

    // kprintf (" - ATAPI] Just get data\n");

    insw(dr->pbase_, buf, words);
    buf += (words * 2);
  }

  // Waiting for an IRQ
  // ATA_WaitIRQ (14);
  // kIrq_Wait(0);

  // Waiting for BSY & DRQ to clear
  while (inb(dr->pbase_ + ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ));

  return 0;
}


/* ----------------------------------------------------------------------- */
static void ATAPI_Detect (struct ATA_Drive *dr)
{
  uint8_t cl = inb(dr->pbase_ + ATA_REG_LBA1);
  uint8_t ch = inb(dr->pbase_ + ATA_REG_LBA2);

  if (!((cl == 0x14 && ch == 0xEB) || (cl == 0x69 && ch == 0x96)))
    return;

  // kprintf ("ATAPI ");
  dr->type_ = IDE_ATAPI;
  outb(dr->pbase_ + ATA_REG_COMMAND, (uint8_t)ATA_CMD_IDENTIFY_PACKET);
  ATA_DELAY;
}


/* ----------------------------------------------------------------------- */
static int ATA_Detect (struct ATA_Drive *dr)
{
  int res, k;
  uint8_t ptr[2048];
  // Select Drive:
  outb(dr->pbase_ + ATA_REG_HDDEVSEL, (uint8_t)dr->disc_);
  ATA_DELAY;
  // Send ATA Identify Command:
  outb(dr->pbase_ + ATA_REG_COMMAND, (uint8_t)ATA_CMD_IDENTIFY);
  ATA_DELAY;
  // Polling:
  res = inb(dr->pbase_ + ATA_REG_STATUS);

  if (res == 0) {
    // kprintf ("no disc \n");
    dr->type_ = IDE_NODISC;
    return 0;
  }

  for (;;) {
    res = inb(dr->pbase_ + ATA_REG_STATUS);

    if ((res & ATA_SR_ERR)) {
      // Probe for ATAPI Devices:
      dr->type_ = IDE_NODISC;
      ATAPI_Detect (dr);

      if (dr->type_ == IDE_NODISC) {
        // kprintf ("unrecognized disc \n");
        return 0;
      }

      break;

    } // else if (!(res & ATA_SR_BSY) && (res & ATA_SR_DRQ)) {

    // kprintf ("ATA ");
    dr->type_ = IDE_ATA;

    break;
  }

  // (V) Read Identification Space of the Device:
  insl (dr->pbase_ + ATA_REG_DATA, ptr, 128);
  // Read Device Parameters:
  dr->signature_ = *((uint16_t *)(ptr + ATA_IDENT_DEVICETYPE));
  dr->capabilities_ = *((uint16_t *)(ptr + ATA_IDENT_CAPABILITIES));
  dr->commandsets_  = *((uint32_t *)(ptr + ATA_IDENT_COMMANDSETS));
  dr->size_ = (dr->commandsets_ & (1 << 26)) ?
              *((uint32_t *)(ptr + ATA_IDENT_MAX_LBA_EXT)) :
              *((uint32_t *)(ptr + ATA_IDENT_MAX_LBA));

  // String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
  for (k = 0; k < 40; k += 2) {
    dr->model_[k] = ptr[ATA_IDENT_MODEL + k + 1];
    dr->model_[k + 1] = ptr[ATA_IDENT_MODEL + k];
  }

  dr->model_[40] = 0; // Terminate String.
  // kprintf (" Size: %d Kb, %s \n", dr->size_ / 2, dr->model_);
  return 1;
}


/* ----------------------------------------------------------------------- */
int ATA_read (kInode_t *ino, void *bucket, size_t length, size_t lba)
{
  struct ATA_Drive *dr = (struct ATA_Drive *)ino->dev_->data_;
  size_t count = length / ino->stat_.block_;
  size_t i;
  int err;

  lba /= ino->stat_.block_;

  // kprintf (" - ATA] read <%d> at lba: %x for %d sectors on %x\n", dr->pbase_, lba, count, bucket);
  if (bucket == NULL)
    return EINVAL;

  switch (dr->type_) {
  case IDE_NODISC:
    return ENODEV;

  case IDE_ATA:

    if (lba + count > dr->size_)
      return ERANGE;

    return ATA_Data (ATA_READ, dr, lba, count, (uint8_t *)bucket);

  case IDE_ATAPI:

    for (i = 0; i < count; ++i) {
      err = ATAPI_Read (dr, lba + i, 1, ((uint8_t *)bucket) + (i * 2048));

      // err = ATAPI_Read2 (dr, lba + i, ((uint8_t*)bucket) + (i * 2048));
      if (err)
        return err;
    }

    return 0;

  default:
    return ENOSYS;
  }
}


/* ----------------------------------------------------------------------- */
int ATA_write (kInode_t *ino, const void *bucket, size_t length, size_t lba)
{
  struct ATA_Drive *dr = (struct ATA_Drive *)ino->dev_->data_;
  size_t count = length / ino->stat_.block_;

  lba /= ino->stat_.block_;
  // kprintf (" - ATA] write <%d> at lba: %x for %d sectors on %x\n", dr->pbase_, lba, count, bucket);

  if (bucket == NULL)
    return EINVAL;

  switch (dr->type_) {
  case IDE_NODISC:
    return ENODEV;

  case IDE_ATAPI:
    return EROFS;

  case IDE_ATA:

    if (lba + count > dr->size_)
      return ERANGE;

    return ATA_Data (ATA_WRITE, dr, lba, count, (uint8_t *)bucket);

  default:
    return ENOSYS;
  }
}


/* ----------------------------------------------------------------------- */
int ATA_mount (kInode_t *dev, const char *name)
{
  int i;
  time_t now = time(NULL);
  SMK_stat_t stat;
  const char *sdNames[] = { "sdA", "sdB", "sdC", "sdD" };

  if (dev != NULL)
    return EIO;

  memset(&stat, 0, sizeof(stat));
  stat.major_ = ATA_No;
  stat.mode_ = S_IFBLK | 0755;
  stat.atime_ = now;
  stat.ctime_ = now;
  stat.mtime_ = now;

  memset (sdx, 0, 4 * sizeof(struct ATA_Drive));
  sdx[0].pbase_ = 0x1f0;
  sdx[0].pctrl_ = 0x3f6;
  sdx[0].disc_ = 0xa0;
  sdx[1].pbase_ = 0x1f0;
  sdx[1].pctrl_ = 0x3f6;
  sdx[1].disc_ = 0xb0;
  sdx[2].pbase_ = 0x170;
  sdx[2].pctrl_ = 0x376;
  sdx[2].disc_ = 0xa0;
  sdx[3].pbase_ = 0x170;
  sdx[3].pctrl_ = 0x376;
  sdx[3].disc_ = 0xb0;

  outb(0x3f6 + ATA_REG_CONTROL - 0x0A, 2);
  outb(0x376 + ATA_REG_CONTROL - 0x0A, 2);

  for (i = 0; i < 4; ++i) {
    if (ATA_Detect(&sdx[i])) {
      stat.minor_ = i;
      stat.block_ = sdx[i].type_ == IDE_ATA ? 512 : 2048;
      stat.length_ = sdx[i].size_ * stat.block_;
      kprintf(" - %s] %s <%s> %s\n", sdNames[i], sdx[i].type_ == IDE_ATA ? "ATA  " : "ATAPI", kpsize(sdx[i].size_), sdx[i].model_);
      create_device(sdNames[i], NULL, &stat, &sdx[i]);
    }
  }

  return 0;
}


/* ----------------------------------------------------------------------- */
void ATA(kDriver_t *driver)
{
  driver->major_ = ATA_No;
  driver->name_ = strdup("ata/atapi");
  driver->mount = ATA_mount;
  driver->read = ATA_read;
  driver->write = ATA_write;
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
