#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/arch.h>
#include <klib/pci.h>

#define INTEL_VEND     0x8086  // Vendor ID for Intel
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217     0x153A  // Device ID for Intel I217
#define E1000_82577LM  0x10EA  // Device ID for Intel 82577LM


// I have gathered those from different Hobby online operating systems
// instead of getting them one by one from the manual
#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818

#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818


#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_RXDCTL       0x3828 // RX Descriptor Control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt



#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        //set link up


#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))


// Transmit Command

#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable


// TCTL Register

#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision

#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun


#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


typedef struct e1000_network_driver e1000_network_driver_t;

struct e1000_network_driver
{
  unsigned char mac_address_[6];
  struct e1000_rx_desc *rx_descs_; // Receive Descriptor Buffers
  struct e1000_tx_desc *tx_descs_; // Transmit Descriptor Buffers
  uint16_t rx_count_; // Count of Receive Descriptor Buffers
  uint16_t tx_count_; // Count of Transmit Descriptor Buffers
  uint16_t rx_cur_; // Current Receive Descriptor Buffer
  uint16_t tx_cur_; // Current Transmit Descriptor Buffer
  uint16_t mx_count_; // Count of Descriptor Buffers
  void *mx_descs_; // Address of Descriptor Buffers
};


PACK(struct e1000_rx_desc {
  volatile uint64_t addr_;
  volatile uint16_t length_;
  volatile uint16_t checksum_;
  volatile uint8_t status_;
  volatile uint8_t errors_;
  volatile uint16_t special_;
});

PACK(struct e1000_tx_desc {
  volatile uint64_t addr_;
  volatile uint16_t length_;
  volatile uint8_t cso_;
  volatile uint8_t cmd_;
  volatile uint8_t status_;
  volatile uint8_t css_;
  volatile uint16_t special_;
});


int e1000_initialize(PCI_device_t *pci);
int e1000_destroy(PCI_device_t *pci);
int e1000_send_packet(const void *data, size_t lg);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static bool e1000_detect_EEProm(PCI_device_t *pci)
{
  int i;
  PCI_write_cmd(pci, REG_EEPROM, 0x1);
  for (i = 0; i < 1000; ++i) {
    if (PCI_read_cmd(pci, REG_EEPROM) & 0x10) {
      return true;
    }
  }
  return false;
}

static uint32_t e1000_read_EEProm(PCI_device_t *pci, uint32_t address)
{
  uint32_t value = 0;
  PCI_write_cmd(pci, REG_EEPROM, (address << 8) | 1);
  while ((value & 0x10) == 0) {
    value = PCI_read_cmd(pci, REG_EEPROM);
  }
  return (value >> 16) & 0xFFFF;
}


#if 0

static void e1000_rxinit(PCI_device_t *pci, e1000_network_driver_t *driver)
{
  int i;
  driver->rx_cur_ = 0;
  driver->rx_count_ = driver->mx_count_ * 3 / 4;
  driver->rx_descs_ = (struct e1000_rx_desc *)driver->mx_descs_;
  uint64_t physical_pointer = Mmu_physical(driver->rx_descs_);

  for(i = 0; i < driver->rx_count_; i++) {
    driver->rx_descs_[i].addr = vmalloc(PAGE_SIZE);
    driver->rx_descs_[i].status = 0;
  }

  /* Inform the card about receive-buffer-descriptor, address: length and count */
  PCI_write_cmd(pci, REG_RXDESCLO, (uint32_t)(physical_pointer >> 32) );
  PCI_write_cmd(pci, REG_RXDESCHI, (uint32_t)(physical_pointer & 0xFFFFFFFF));
  PCI_write_cmd(pci, REG_RXDESCLEN, driver->rx_count_ * 16);
  PCI_write_cmd(pci, REG_RXDESCHEAD, 0);
  PCI_write_cmd(pci, REG_RXDESCTAIL, driver->rx_count_ - 1);

  /* Configure receive flags */
  PCI_write_cmd(pci, REG_RCTRL, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE |
    RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_2048);
}

static void e1000_txinit(PCI_device_t *pci, e1000_network_driver_t *driver)
{
  int i;
  driver->tx_cur_ = 0;
  driver->tx_count_ = driver->mx_count_ / 4;
  driver->tx_descs_ = (struct e1000_tx_desc *)__PTR_ADD(driver->mx_descs_, 16 * 3 * driver->tx_count_);
  uint64_t physical_pointer = Mmu_physical(driver->tx_descs_);

  for(i = 0; i < driver->tx_count_; i++) {
    driver->tx_descs_[i].addr = 0;
    driver->tx_descs_[i].cmd = 0;
    driver->tx_descs_[i].status = TSTA_DD;
  }

  /* Inform the card about transfert-buffer-descriptor: address, length and count */
  PCI_write_cmd(pci, REG_TXDESCHI, (uint32_t)(physical_pointer >> 32) );
  PCI_write_cmd(pci, REG_TXDESCLO, (uint32_t)(physical_pointer & 0xFFFFFFFF));
  PCI_write_cmd(pci, REG_TXDESCLEN, driver->tx_count_ * 16);
  PCI_write_cmd(pci, REG_TXDESCHEAD, 0);
  PCI_write_cmd(pci, REG_TXDESCTAIL, 0);

  /* Configure receive flags */
  PCI_write_cmd(pci, REG_TCTRL,  TCTL_EN
      | TCTL_PSP
      | (15 << TCTL_CT_SHIFT)
      | (64 << TCTL_COLD_SHIFT)
      | TCTL_RTLC);

  // This line of code overrides the one before it but I left both to highlight
  // that the previous one works with e1000 cards, but for the e1000e cards
  // you should set the TCTRL register as follows.
  // For detailed description of each bit, please refer to the Intel Manual.
  // In the case of I217 and 82577LM packets will not be sent if the TCTRL
  // is not configured using the following bits.
  PCI_write_cmd(pci, REG_TCTRL,  0b0110000000000111111000011111010);
  PCI_write_cmd(pci, REG_TIPG,  0x0060200A);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


int e1000_send_packet(PCI_device_t *pci, e1000_network_driver_t *driver, const void *data, size_t lg)
{
  struct e1000_tx_desc *tx_desc = &driver->tx_descs_[driver->tx_cur_];
  /* Send the buffer to the network card. */
  tx_desc->addr_ = (uint64_t)data;
  tx_desc->length_ = lg;
  tx_desc->cmd_ = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS;
  tx_desc->status_ = 0;

  PCI_write_cmd(pci, REG_TXDESCTAIL, driver->tx_cur_++);
  if (driver->tx_cur_ >= driver->tx_count_) {
    // TODO - That's what I found, but I have some doubt on the algorithm.
    driver->tx_cur_ = 0;
  }

  // TODO - Do we have to wait, see how tails is working !?
  while(!(tx_descs[old_cur]->status & 0xff));
  return 0;
}

int e1000_receive_packet(PCI_device_t *pci, e1000_network_driver_t *driver)
{
  for (;;) {
    struct e1000_rx_desc *rx_desc = &driver->rx_descs_[driver->rx_cur_];
    if ((rx_desc->status_ & 0x1) == 0) {
      return 0;
    }

    /* Transfert the ownership of the buffer to the network stack. */
    Network_received_packet(rx_desc->addr_, rx_desc->length_);

    /* Allocate a new buffer to use, and mark this one as available again. */
    rx_desc->addr_ = vmalloc(PAGE_SIZE);
    rx_desc->status = 0;
    PCI_write_cmd(pci, REG_RXDESCTAIL, driver->rx_cur_++);
    if (driver->rx_cur_ >= driver->rx_count_) {
      // TODO - That's what I found, but I have some doubt on the algorithm.
      driver->rx_cur_ = 0;
    }
  }
}

#endif

int e1000_read_IRQ(PCI_device_t *pci)
{
  uint32_t status = PCI_read_cmd(pci, 0xc0);
  if(status & 0x04) {
    // e1000_start_link();
  } else if(status & 0x10) {
    // good threshold
  } else if(status & 0x80) {
    // e1000_receive_packet(pci, (e1000_network_driver_t)pci->data_);
  }
  return 0;
}


int e1000_initialize(PCI_device_t *pci)
{
  int i;
  uint32_t tmp;
  struct e1000_network_driver *driver;
  unsigned char mac_address[6];

  /* Look for the name of the device */
  switch (pci->device_id_) {
    case E1000_DEV: pci->dev_name_ = "E1000 Virtual host"; break;
    case E1000_I217: pci->dev_name_ = "E1000 Intel I217"; break;
    case E1000_82577LM: pci->dev_name_ = "E1000 Intel 82577LM"; break;
    default:
      return -1;
  }

  if (e1000_detect_EEProm(pci)) {
    for (i = 0; i < 3; ++i) {
      tmp = e1000_read_EEProm(pci,i);
      mac_address[2 * i] = tmp & 0xff;
      mac_address[2 * i + 1] = tmp >> 8;
    }
  } else {
    if (*(uint32_t *)(pci->mmio_ + 0x5400) == 0) {
      return -1;
    }

    for (i = 0; i < 6; ++i) {
      mac_address[i] = ((uint8_t *)pci->mmio_ + 0x5400)[i];
    }
  }

  for(int i = 0; i < 0x80; i++) {
    PCI_write_cmd(pci, 0x5200 + i * 4, 0);
  }

  driver = (e1000_network_driver_t*)malloc(sizeof(e1000_network_driver_t));
  pci->data_ = driver;
  if (IRQ_register(pci->irq_, (IRQ_handler_f)e1000_read_IRQ, pci) != 0) {
    free(driver);
    return -1;
  }

  /* Autorize device to send IRQ. */
  PCI_write_cmd(pci, REG_IMASK, 0x1F6DC);
  PCI_write_cmd(pci, REG_IMASK, 0xff & ~4);
  PCI_read_cmd(pci, 0xc0);

  /* Allocate buffer for transfert and receive packets. */
  driver->mx_count_ = PAGE_SIZE / 16;
  // driver->mx_descs_ = vmalloc(PAGE_SIZE);
  // e1000_rxinit(pci, driver);
  // e1000_txinit(pci, driver);

  memcpy(driver->mac_address_, mac_address, 6);
  kprintf("E1000 card started, using MAC address %02x-%02x-%02x-%02x-%02x-%02x\n",
    mac_address[0], mac_address[1], mac_address[2],
    mac_address[3], mac_address[4], mac_address[5]);
  return 0;
}

