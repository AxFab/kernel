
global IntEx00_Handler
global IntEx01_Handler
global IntEx02_Handler
global IntEx03_Handler
global IntEx04_Handler
global IntEx05_Handler
global IntEx06_Handler
global IntEx07_Handler
global IntEx08_Handler
global IntEx09_Handler
global IntEx0A_Handler
global IntEx0B_Handler
global IntEx0C_Handler
global IntEx0D_Handler
global IntEx0E_Handler

global IRQ0_Handler
global IRQ1_Handler
global IRQ2_Handler
global IRQ3_Handler
global IRQ4_Handler
global IRQ5_Handler
global IRQ6_Handler
global IRQ7_Handler
global IRQ8_Handler
global IRQ9_Handler
global IRQ10_Handler
global IRQ11_Handler
global IRQ12_Handler
global IRQ13_Handler
global IRQ14_Handler
global IRQ15_Handler

global SysCall_Handler
global Interrupt_Handler

extern sys_exception
extern sys_page_fault
extern sys_irq
extern sys_enter

; ====================================

%macro SAVE_REGS 0
    pushad
    push ds
    push es
    push fs
    push gs
    push ebx
    mov bx,0x10
    mov ds,bx
    pop ebx
%endmacro

%macro LOAD_REGS 0
    pop gs
    pop fs
    pop es
    pop ds
    popad
%endmacro

%macro INT_EX_HANDLER 1
    SAVE_REGS
    push esp
    push %1
    call sys_exception
    add esp, 8
    LOAD_REGS
    iret
%endmacro

%macro IRQ_HANDLER 1
    SAVE_REGS
    push esp
    push %1
    call sys_irq
    add esp, 8
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret
%endmacro

; ====================================

Interrupt_Handler:
    SAVE_REGS
    push esp
    push dword -1
    call sys_exception
    add esp, 8
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret

IntEx00_Handler:
    INT_EX_HANDLER 0x00
IntEx01_Handler:
    INT_EX_HANDLER 0x01
IntEx02_Handler:
    INT_EX_HANDLER 0x02
IntEx03_Handler:
    INT_EX_HANDLER 0x03
IntEx04_Handler:
    INT_EX_HANDLER 0x04
IntEx05_Handler:
    INT_EX_HANDLER 0x05
IntEx06_Handler:
    INT_EX_HANDLER 0x06
IntEx07_Handler:
    INT_EX_HANDLER 0x07
IntEx08_Handler:
    INT_EX_HANDLER 0x08
IntEx09_Handler:
    INT_EX_HANDLER 0x09
IntEx0A_Handler:
    INT_EX_HANDLER 0x0a
IntEx0B_Handler:
    INT_EX_HANDLER 0x0b
IntEx0C_Handler:
    INT_EX_HANDLER 0x0c
IntEx0D_Handler:           ; General protection fault
    INT_EX_HANDLER 0x0d
IntEx0E_Handler:           ; Page fault
    SAVE_REGS
    push esp
    push dword [esp + 52]            ; get error code
    mov eax, cr2
    push eax
    call sys_page_fault
    add esp, 12
    LOAD_REGS
    add esp, 4
    iret
IntEx0F_Handler:
    INT_EX_HANDLER 0x0f

; ====================================


IRQ0_Handler:
  IRQ_HANDLER 0
IRQ1_Handler:
  IRQ_HANDLER 1
IRQ2_Handler:
  IRQ_HANDLER 2
IRQ3_Handler:
  IRQ_HANDLER 3
IRQ4_Handler:
  IRQ_HANDLER 4
IRQ5_Handler:
  IRQ_HANDLER 5
IRQ6_Handler:
  IRQ_HANDLER 6
IRQ7_Handler:
  IRQ_HANDLER 7
IRQ8_Handler:
  IRQ_HANDLER 8
IRQ9_Handler:
  IRQ_HANDLER 9
IRQ10_Handler:
  IRQ_HANDLER 10
IRQ11_Handler:
  IRQ_HANDLER 11
IRQ12_Handler:
  IRQ_HANDLER 12
IRQ13_Handler:
  IRQ_HANDLER 13
IRQ14_Handler:
  IRQ_HANDLER 14
IRQ15_Handler:
  IRQ_HANDLER 15

; ====================================

SysCall_Handler:
    SAVE_REGS
    push esp
    mov ebp, esp
    push dword [ebp + 48]
    call sys_enter
    add esp, 8
    LOAD_REGS
    iret
