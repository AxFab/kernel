#include <smkos/kernel.h>
#include <smkos/pio.h>
#include <smkos/_compiler.h>

#define INTGATE  0x8E00     /**< used for interruptions */
#define TRAPGATE 0xEF00     /**< used for system calls */

// ===========================================================================

PACK(struct x86_GdtEntry {
  uint16_t lim0_15;
  uint16_t base0_15;
  uint8_t base16_23;
  uint8_t access;
  uint8_t lim16_19 : 4;
  uint8_t other : 4;
  uint8_t base24_31;
});

PACK(struct x86_IdtEntry {
  uint16_t offset0_15;
  uint16_t segment;
  uint16_t type;
  uint16_t offset16_31;
});

struct x86_TaskSs {
  uint16_t    previous_task, __previous_task_unused;
  uint32_t    esp0;
  uint16_t    ss0, __ss0_unused;
  uint32_t    esp1;
  uint16_t    ss1, __ss1_unused;
  uint32_t    esp2;
  uint16_t    ss2, __ss2_unused;
  uint32_t    cr3;
  uint32_t    eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  uint16_t    es, __es_unused;
  uint16_t    cs, __cs_unused;
  uint16_t    ss, __ss_unused;
  uint16_t    ds, __ds_unused;
  uint16_t    fs, __fs_unused;
  uint16_t    gs, __gs_unused;
  uint16_t    ldt_selector, __ldt_sel_unused;
  uint16_t    debug_flag, io_map;
};

// ===========================================================================
void IntEx00_Handler();
void IntEx01_Handler();
void IntEx02_Handler();
void IntEx03_Handler();
void IntEx04_Handler();
void IntEx05_Handler();
void IntEx06_Handler();
void IntEx07_Handler();
void IntEx08_Handler();
void IntEx09_Handler();
void IntEx0A_Handler();
void IntEx0B_Handler();
void IntEx0C_Handler();
void IntEx0D_Handler();
void IntEx0E_Handler();

// ===========================================================================
void IRQ0_Handler();
void IRQ1_Handler();
void IRQ2_Handler();
void IRQ3_Handler();
void IRQ4_Handler();
void IRQ5_Handler();
void IRQ6_Handler();
void IRQ7_Handler();
void IRQ8_Handler();
void IRQ9_Handler();
void IRQ10_Handler();
void IRQ11_Handler();
void IRQ12_Handler();
void IRQ13_Handler();
void IRQ14_Handler();
void IRQ15_Handler();

void SysCall_Handler();
void Interrupt_Handler();

struct x86_TaskSs* i386_TssAddress = (struct x86_TaskSs*)0x1000;

/* ----------------------------------------------------------------------- */
/** Write an GDT entry on the table */
static void GDT_entry (int number, uint32_t base, uint32_t limit, int access, int other)
{
  struct x86_GdtEntry *ptr = (struct x86_GdtEntry *)(0);
  ptr += number;
  ptr->lim0_15 = (limit & 0xffff);
  ptr->base0_15 = (base & 0xffff);
  ptr->base16_23 = (base & 0xff0000) >> 16;
  ptr->access = access;
  ptr->lim16_19 = (limit & 0xf0000) >> 16;
  ptr->other = (other & 0xf);
  ptr->base24_31 = (base & 0xff000000) >> 24;
}

/* ----------------------------------------------------------------------- */
static void IDT_entry (int number, int segment, uint32_t address, int type)
{
  struct x86_IdtEntry *ptr = (struct x86_IdtEntry *)(0x800);
  ptr += number;
  ptr->offset0_15 = (address & 0xffff);
  ptr->segment = segment;
  ptr->type = type;
  ptr->offset16_31 = (address & 0xffff0000) >> 16;
}

/* ----------------------------------------------------------------------- */
static void PIC_init()
{
  /* Initialisation de ICW1 */
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  /* Initialisation de ICW2 */
  outb(0x21, 0x20); /* vecteur de depart = 32 */
  outb(0xA1, 0x70); /* vecteur de depart = 96 */

  /* Initialisation de ICW3 */
  outb(0x21, 0x04);
  outb(0xA1, 0x02);

  /* Initialisation de ICW4 */
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  /* masquage des interruptions */
  outb(0x21, 0x0);
  outb(0xA1, 0x0);
}



// ===========================================================================
void cpu_init_table ()
{
  int i;

  // GDT - Global Descriptor Table
  GDT_entry(0, 0, 0, 0, 0);                         // Empty
  GDT_entry(1, 0x0, 0xfffff, 0x9B, 0x0D);           // kernel Code
  GDT_entry(2, 0x0, 0xfffff, 0x93, 0x0D);           // kernel Data
  GDT_entry(3, 0x0, 0x00000, 0x97, 0x0D);           // kernel Stack
  GDT_entry(4, 0x0, 0xfffff, 0xff, 0x0D);           // user Code
  GDT_entry(5, 0x0, 0xfffff, 0xf3, 0x0D);           // user Data
  GDT_entry(6, 0x0, 0x00000, 0xf7, 0x0D);           // user Stack
  GDT_entry(7, (uint32_t)i386_TssAddress, 0x67, 0xe9, 0x00);  // TSS

  // TSS - Task State Segment
  i386_TssAddress->debug_flag = 0x00;
  i386_TssAddress->io_map = 0x00;
  i386_TssAddress->esp0 = (0x7000 - 4); // TODO Kernel Stack !
  i386_TssAddress->ss0 = 0x18;  // TODO Kernel stack segment

  // IDT - Interupt Descriptor Table
  for (i = 0; i < 256; ++i)
    IDT_entry(i, 0x08, (uint32_t)Interrupt_Handler, INTGATE);

  // Hardware Exception
  IDT_entry(0x00, 0x08, (uint32_t)IntEx00_Handler, TRAPGATE);
  IDT_entry(0x01, 0x08, (uint32_t)IntEx01_Handler, TRAPGATE);
  IDT_entry(0x02, 0x08, (uint32_t)IntEx02_Handler, TRAPGATE);
  IDT_entry(0x03, 0x08, (uint32_t)IntEx03_Handler, TRAPGATE);
  IDT_entry(0x04, 0x08, (uint32_t)IntEx04_Handler, TRAPGATE);
  IDT_entry(0x05, 0x08, (uint32_t)IntEx05_Handler, TRAPGATE);
  IDT_entry(0x06, 0x08, (uint32_t)IntEx06_Handler, TRAPGATE);
  IDT_entry(0x07, 0x08, (uint32_t)IntEx07_Handler, TRAPGATE);
  IDT_entry(0x08, 0x08, (uint32_t)IntEx08_Handler, TRAPGATE);
  IDT_entry(0x09, 0x08, (uint32_t)IntEx09_Handler, TRAPGATE);
  IDT_entry(0x0a, 0x08, (uint32_t)IntEx0A_Handler, TRAPGATE);
  IDT_entry(0x0b, 0x08, (uint32_t)IntEx0B_Handler, TRAPGATE);
  IDT_entry(0x0c, 0x08, (uint32_t)IntEx0C_Handler, TRAPGATE);
  IDT_entry(0x0d, 0x08, (uint32_t)IntEx0D_Handler, TRAPGATE);
  IDT_entry(0x0e, 0x08, (uint32_t)IntEx0E_Handler, TRAPGATE);

  // Hardware Interrupt Request - Master
  IDT_entry(0x20, 0x08, (uint32_t)IRQ0_Handler, INTGATE);
  IDT_entry(0x21, 0x08, (uint32_t)IRQ1_Handler, INTGATE);
  IDT_entry(0x22, 0x08, (uint32_t)IRQ2_Handler, INTGATE);
  IDT_entry(0x23, 0x08, (uint32_t)IRQ3_Handler, INTGATE);
  IDT_entry(0x24, 0x08, (uint32_t)IRQ4_Handler, INTGATE);
  IDT_entry(0x25, 0x08, (uint32_t)IRQ5_Handler, INTGATE);
  IDT_entry(0x26, 0x08, (uint32_t)IRQ6_Handler, INTGATE);
  IDT_entry(0x27, 0x08, (uint32_t)IRQ7_Handler, INTGATE);

  IDT_entry(0x70, 0x08, (uint32_t)IRQ8_Handler, INTGATE);
  IDT_entry(0x71, 0x08, (uint32_t)IRQ9_Handler, INTGATE);
  IDT_entry(0x72, 0x08, (uint32_t)IRQ10_Handler, INTGATE);
  IDT_entry(0x73, 0x08, (uint32_t)IRQ11_Handler, INTGATE);
  IDT_entry(0x74, 0x08, (uint32_t)IRQ12_Handler, INTGATE);
  IDT_entry(0x75, 0x08, (uint32_t)IRQ13_Handler, INTGATE);
  IDT_entry(0x76, 0x08, (uint32_t)IRQ14_Handler, INTGATE);
  IDT_entry(0x77, 0x08, (uint32_t)IRQ15_Handler, INTGATE);

  IDT_entry(0x30, 0x08, (uint32_t)SysCall_Handler, TRAPGATE);

  PIC_init ();
}
