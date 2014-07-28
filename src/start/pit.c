#include <kernel/core.h>
#include <kernel/cpu.h>

#define MICROSEC_IN_SEC (1000 * 1000)
#define PIT_CH0   0x40  // Channel 0 data port (read/write)
#define PIT_CH1   0x41  // Channel 1 data port (read/write)
#define PIT_CH2   0x42  // Channel 2 data port (read/write)
#define PIT_CMD   0x43  // Mode/Command register (write only, a read is ignored)

uint32_t PIT_Period = 0;
uint32_t PIT_Frequency = 0;

void PIT_Initialize (uint32_t frequency)
{
  uint32_t divisor = 1193180 / frequency; /* Calculate our divisor */
  divisor = MIN (65536, MAX(1, divisor));

  PIT_Frequency = 1193180 / divisor;
  PIT_Period = MICROSEC_IN_SEC / PIT_Frequency;

  outb(0x43, 0x36);             /* Set our command byte 0x36 */
  outb(0x40, divisor & 0xff);   /* Set low byte of divisor */
  outb(0x40, (divisor >> 8) & 0xff);     /* Set high byte of divisor */

  kprintf ("Set PIT timer : Fq %d Hz, Period: %d us, <rate %d>\n", PIT_Frequency, PIT_Period, divisor);
}


