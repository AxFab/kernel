
STARTUP(obj/crt0.o)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
  . = 0x1000000;
  .text ALIGN(0x1000) : { *(.text) }
  .data ALIGN(0x1000): { *(.data) }
  .bss : { *(.bss) }
}

