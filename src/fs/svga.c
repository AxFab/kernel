
#include <smkos/kfs.h>


#define SVGA_IO_CRTC_ADDRESS    0x3b4
#define SVGA_IO_CRTC_DATA       0x3b5


/* Port offsets, relative to BASE */
#define SVGA_INDEX_PORT         0x0
#define SVGA_VALUE_PORT         0x1
#define SVGA_BIOS_PORT          0x2
#define SVGA_IRQSTATUS_PORT     0x8


struct SVGA_Device {
  int ioBase_;
};

/* ======================================================================= */
// static inline int SVGA_getRegister(struct SVGA_Device *dr, int idx)
// {
//   outl(dr->ioBase_ + SVGA_INDEX_PORT, idx);
//   return inl(dr->ioBase_ + SVGA_VALUE_PORT);
// }

// static inline void SVGA_setRegister(struct SVGA_Device *dr, int idx)
// {
//   outl(dr->ioBase_ + SVGA_INDEX_PORT, idx);
//   outl(dr->ioBase_ + SVGA_VALUE_PORT, idx);
// }

struct VGA_Frame {
  size_t address_;
  int width_;
  int height_;
  int depth_;
} FB0;


void VGA_Info(size_t add, int width, int height, int depth)
{
  FB0.address_ = add;
  FB0.width_ = width;
  FB0.height_ = height;
  FB0.depth_ = depth;
}


int VGA_map (kInode_t *ino, size_t offset, page_t *phys)
{
  struct VGA_Frame *fr = (struct VGA_Frame *)ino->dev_->data_;
  *phys = fr->address_ + offset;
  return 0;
}



/* ----------------------------------------------------------------------- */
int VGA_mount (kInode_t *dev, const char *name)
{
  time_t now = time(NULL);
  struct SVGA_Device *device;
  SMK_stat_t stat;

  // device = kalloc(sizeof(struct SVGA_Device));
  // device->ioBase_ = SVGA_IO_CRTC_ADDRESS;


  if (dev != NULL)
    return ENODEV;

  memset(&stat, 0, sizeof(stat));

  if (FB0.address_ != 0) {
    stat.major_ = VGA_No;
    stat.mode_ = S_IFBLK | 0755;
    stat.atime_ = now;
    stat.ctime_ = now;
    stat.mtime_ = now;
    stat.length_ = FB0.width_ * FB0.height_ * FB0.depth_;
    stat.block_ = FB0.width_ * FB0.depth_;

    create_device("Fb0", NULL, &stat, &FB0);
  }

  // kprintf (" - VGA] Try to mount vga framebuffer\n");
  // SVGA_setRegister(device);
  // kprintf ("   -> 0 %d \n", SVGA_getRegister(device, 0));
  // kprintf ("   -> 1 %d \n", SVGA_getRegister(device, 1));
  // kprintf ("   -> 2 %d \n", SVGA_getRegister(device, 2));
  // kprintf ("   -> 3 %d \n", SVGA_getRegister(device, 3));
  // kprintf ("   -> 5 %d \n", SVGA_getRegister(device, 5));

  return 0;
}

/* ----------------------------------------------------------------------- */
void VGA(kDriver_t *driver)
{
  driver->major_ = VGA_No;
  driver->name_ = strdup("VGA");
  driver->mount = VGA_mount;
  driver->map = VGA_map;
}

