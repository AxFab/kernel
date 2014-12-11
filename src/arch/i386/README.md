
#### x86 Memory mapping
  Base      |  Limit     |  Size  |  Note
------------|------------|--------|------------
 0x00000000 | 0x0001ffff | 128 Kb | Kernel tables
 0x00020000 | 0x0007ffff | 384 Kb | Kernel code
 0x00080000 | 0x0009ffff | 128 Kb | RAM page bitmap
 0x000A0000 | 0x000fffff | 384 Kb | Hardware
 0x00100000 | 0x00100fff |   4 Kb | APIC
 0x00101000 | 0x00101fff |   4 Kb | Kernel stack CPU #1
 0x00102000 | 0x00102fff |   4 Kb | Kernel stack CPU #2
 0x00103000 | 0x0013ffff |  x255  | Kernel stack CPU #?
 ...        | ...        | 768 Kb | Empty
 0x00200000 | ...        |   1 Mb | Kernel kSYS structure
 0x00300000 | ...        |   1 Mb | Kernel kUSP structure
 0x00400000 | 0xe03fffff | 3.5 Gb | USER SPACE
 0xe0400000 | 0xffbfffff | 504 Mb | Kernel heap / slab
 0xffc00000 | 0xffffffff |   4 Mb | Kernel directory page mirroring

##### Kernel tables (first Mb)
  Base      |  Limit     |  Size      |  Note
------------|------------|------------|--------
 0x00000000 | 0x00000037 |   56 bytes | GDT entry
 0x00000038 | 0x000007d7 | 1960 bytes | TSS, one for each CPU (x245)
 0x000007e0 | 0x000007ff |   32 bytes | Multi-core vars 
 0x00000800 | 0x00000fff | 2048 bytes | IDT
 0x00001000 | 0x00001fff |   4 Kb |   (OLD TSS) 
 0x00002000 | 0x00002fff |   4 Kb | GDE Kernel directory page
 0x00003000 | 0x00003fff |   4 Kb | PFE Kernel table page 0
 0x00004000 | 0x00004fff |   4 Kb | PFE Kernel table page 1
 0x00005000 | 0x00005fff |   4 Kb | Temporary page (ALERT!)
 0x00006000 | 0x00006fff |   4 Kb | Kernel stack CPU #0
 0x00007000 | 0x0000ffff |  36 Kb | Kernel logs
 0x00010000 | 0x0001ffff |  64 kb | __ unused
 0x00020000 | 0x0007ffff | 384 Kb |  Kernel code
 0x00080000 | 0x0009ffff | 128 Kb | RAM page bitmap
 0x000A0000 | 0x000fffff | 384 Kb | Hardware

__CPU__:

 - CPU_GDT
 - CPU_LOCK
 - CPU_STACK
 - CPU_COUNT

__IDT__: 

  - [00..0f] CPU Exception
  - [20..27,70..77] IRQ
  - [30] Syscall

> __*__ For the moment the address 0x400000 map the screen...
> __*__ We use 0x5000 for temporary map, but this is not multi-core.



  Hardware Exception
    0x00  Division by zero
    0x01  Debugger
    0x02  NMI
    0x03  Breakpoint
    0x04  Overflow
    0x05  Bounds
    0x06  Invalid Opcode
    0x07  Coprocessor not available
    0x08  Double fault
    0x09  Coprocessor Segment Overrun (386 or earlier only)
    0x0A  Invalid Task State Segment
    0x0B  Segment not present
    0x0C  Stack Fault
    0x0D  General protection fault
    0x0E  Page fault
    0x0F  reserved
    0x10  Math Fault
    0x11  Alignment Check
    0x12  Machine Check
    0x13  SIMD Floating-Point Exception

  Hardware Interrupt Request
    IRQ 0 (0x20) : System Clock
    IRQ 1 : Keyboard
    IRQ 2 : N/A
    IRQ 3 : Serial port (COM2/COM4)
    IRQ 4 : Serial port (COM1/COM3)
    IRQ 5 : LPT2 (sound card)
    IRQ 6 : Floppy drive
    IRQ 7 : Parallel port (LPT1)
    IRQ 8 (0x70) : Real Time Clock (CMOS)
    IRQ 9 : N/A (PCI)
    IRQ 10 : N/A
    IRQ 11 : N/A (USB)
    IRQ 12 : N/A (PS/2)
    IRQ 13 : Math Coprocessor
    IRQ 14 : Primary HDD
    IRQ 15 : Secondary HDD

