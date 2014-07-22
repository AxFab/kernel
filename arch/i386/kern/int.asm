use32

global kcpu_Default, kcpu_Clock, kcpu_KBoard, kcpu_SysCall
extern kInt_Default, kInt_Clock, kInt_KBoard, kInt_SysCall

global kcpu_PageFault, kcpu_Protect
extern kInt_PageFault, kInt_Protect


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

global makeInterrupt, __delay

__delay:
    push ecx
    mov ecx, 0x5000000
  .s:
    loop .s
    pop ecx
    ret

makeInterrupt:
    int 0x15

kcpu_Default:
    SAVE_REGS
    call kInt_Default
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret

; 0x0E - Page Fault
kcpu_PageFault:
    SAVE_REGS
    mov eax, cr2
    push eax
    call kInt_PageFault
    pop eax
    LOAD_REGS
    add esp, 4
    iret

; 0x0F - Global Protection
kcpu_Protect:
    SAVE_REGS
    mov eax, cr2
    push eax
    call kInt_Protect
    pop eax
    LOAD_REGS
    add esp, 4
    iret

; 0x20 - Hardware Clock
kcpu_Clock:
    SAVE_REGS
    call kInt_Clock
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret

; 0x21 - Keyboard
kcpu_KBoard:
    SAVE_REGS
    call kInt_KBoard
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret

; 0x70 - CMOS [1024Hz clock]
global kcpu_CMOS
extern kInt_CMOS
kcpu_CMOS:
    SAVE_REGS
    call kInt_CMOS
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret


; 0x30 - System Calls
kcpu_SysCall:
    SAVE_REGS
    push esp
    call kInt_SysCall
    add esp, 4
    LOAD_REGS
    iret

global kcpu_Irq15
extern kIrq_Do
kcpu_Irq15:
    SAVE_REGS
    ;push dword 15
    call kIrq_Do
    ;pop eax
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret

;kcpu_LaunchTask:
;    push ebp
;    mov ebp, esp
;    sub esp, 0x10
;    mov edx, [ebp + 8]
;    ;mov [ThisThread], edx
;    mov eax, [edx + Thread_Dir]
;    mov cr3, eax
;    mov eax, edx
;    add eax, Thread_Regs
;    mov dword [esp], 0x00800000 - 0x100
;    mov [esp + 4], eax
;    mov dword [esp + 8], SizeOf_Regs
;    call memcpy
;    mov ebx, [eax + Regs_Flags]
;    or ebx, 0x200
;    and ebx, 0xFFFFBFFF
;    mov [eax + Regs_Flags], ebx
;    mov esp, 0x00800000 - 0x100
;    mov al, 0x20
;    out 0x20, al
;    LOAD_REGS
;    sti
;    ;jmp $
;    iret



; ============================================================================

extern kprintf

%macro INT_EX_HANDLER 1
    SAVE_REGS
    push dword %1
    push dword IntEx_Message
    call kprintf
    LOAD_REGS
    iret
%endmacro

IntEx_Message:
    db "Interrupt exception <%2x> ", 10, 0

global IntEx00_Handler
IntEx00_Handler:
    INT_EX_HANDLER 0x00

global IntEx01_Handler
IntEx01_Handler:
    INT_EX_HANDLER 0x01

global IntEx02_Handler
IntEx02_Handler:
    INT_EX_HANDLER 0x02

global IntEx03_Handler
IntEx03_Handler:
    INT_EX_HANDLER 0x03

global IntEx04_Handler
IntEx04_Handler:
    INT_EX_HANDLER 0x04

global IntEx05_Handler
IntEx05_Handler:
    INT_EX_HANDLER 0x05

global IntEx06_Handler
IntEx06_Handler:
    INT_EX_HANDLER 0x06

global IntEx07_Handler
IntEx07_Handler:
    INT_EX_HANDLER 0x07


extern kInt_Look
global kcpu_Look
kcpu_Look:
    SAVE_REGS
    call kInt_Look
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret


; ========================================================
; IDE BUS 

global IRQ14_handler, IRQ15_handler
extern kCpu_IRQ

IRQ14_handler:
    SAVE_REGS
    push esp
    push 14
    call kCpu_IRQ
    add esp, 8
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret


IRQ15_handler:
    SAVE_REGS
    push esp
    push 15
    call kCpu_IRQ
    add esp, 8
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret
