use32

%define SIZEOF_CPU_REGS 5

%define GRUB_MAGIC1     0x1BADB002
%define GRUB_MAGIC2     0x2BADB002
%define GRUB_FLAGS      0x00010003; 0x00010007

extern code, bss, end

%define MEM_KSTACK_PTR  0x7000

%define SGMT_KRN_CODE     0x08
%define SGMT_KRN_DATA     0x10
%define SGMT_KRN_STACK    0x18
%define _x86_uCode_Sgmt     0x23
%define _x86_uData_Sgmt     0x2d
%define _x86_uStack_Sgmt    0x33
%define _x86_TSS_Sgmt       0x38


global _start
global cpuid
global cpu_halt_

extern grub_initialize
extern kernel_start
extern cpu_init_table

_start:
    mov esp, MEM_KSTACK_PTR - 0x10
    jmp start

; MultiBootHeader --------------------------
align 4
mboot:
    dd GRUB_MAGIC1
    dd GRUB_FLAGS
    dd - GRUB_FLAGS - GRUB_MAGIC1

    dd mboot
    dd code
    dd bss
    dd end
    dd start

    ; dd _start       ; header_addr
    ; dd _start       ; load_addr
    ; dd 0            ; load_end_addr
    ; dd 0
    ; dd start

    dd 1            ; mode
    dd 800           ; width
    dd 600           ; height
    dd 0            ; depth

align 16
start:
; Start ------------------------------------
    cli
    mov esp, MEM_KSTACK_PTR - 0x10
    cmp eax, GRUB_MAGIC2
    jmp .grubLoad

  .unknowLoader:
    mov byte [0xB8000], 'U'
    mov byte [0xB8001], 0x57
    mov byte [0xB8002], 'n'
    mov byte [0xB8003], 0x57
    mov byte [0xB8004], '!'
    mov byte [0xB8005], 0x57
    mov byte [0xB8006], 0x00
    jmp .failed

  .errorLoader:
    mov byte [0xB8000], 'E'
    mov byte [0xB8001], 0x57
    mov byte [0xB8002], 'r'
    mov byte [0xB8003], 0x57
    mov byte [0xB8004], 0x00

  .failed:
    hlt
    jmp $ 


; Grub -------------------------------------
  .grubLoad:

    push ebx
    call grub_initialize
    pop ebx
    test eax, eax
    jnz .errorLoader

    mov byte [0xB8000], 'G'
    mov byte [0xB8001], 0x57
    mov byte [0xB8002], 'o'
    mov byte [0xB8003], 0x57
    mov byte [0xB8004], 0x00


; Initialize CPU ---------------------------
startup:
    xor eax, eax
    mov edi, eax
    mov ecx, 2048
    rep stosd

    call cpu_init_table
    
    mov esp, MEM_KSTACK_PTR - 0x10
    lgdt [.gdtregs]
    jmp SGMT_KRN_CODE:.reloadCS
  .reloadCS:
    mov ax, SGMT_KRN_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov byte [0xB8004], '.'
    mov byte [0xB8005], 0x57
    

    lidt [.idtregs]
    mov ax, _x86_TSS_Sgmt
    ltr ax

    mov byte [0xB8006], '.'
    mov byte [0xB8007], 0x57
    
    mov eax, 0x2000 ; PAGE kernel
    mov cr3, eax
    mov eax, cr0
    or eax, (1 << 31) ; CR0 31b to activate mmu
    mov cr0, eax

    mov byte [0xB8008], '.'
    mov byte [0xB8009], 0x57
    
    call kernel_start
    hlt
    jmp $ 

    sti
    jmp $


; Data -------------------------------------
align 4
  .gdtregs:
    dw 0xF8, 0, 0, 0
  .idtregs:
    dw 0x7F8, 0x800, 0, 0


; ------------------------------------------

; %include "arch/i386/kern/io.asm"
; %include "arch/i386/kern/pit.asm"
; %include "arch/i386/kern/interupt.asm"
; align 4096
; %include "arch/i386/kern/ap_boot.asm"
; align 4096



global kCpu_SwitchContext
global kCpu_Switch, kCpu_Switch2
extern mmu_newdir, kTty_HexChar, kDBG


; =============================================
; =============================================


kCpu_Switch:
kCpu_SwitchContext:

kcpuswitch:

    cli
    push ebp
    mov ebp, esp
    mov esp, 0x6800

  ; Copy REGS on stack
    mov ax, ds
    mov es, ax
    sub esp, SIZEOF_CPU_REGS
    mov ecx, SIZEOF_CPU_REGS
    mov esi, [ebp + 8]
    mov edi, esp
    rep movsb

    ; Fix ESP
    add esp, 0x10
    mov [esp + 3*4], esp
    sub esp, 0x10

    ; Fix EFLAGS
    mov eax, [esp + 14*4]
    or eax, 0x200
    and eax, 0xffffbfff
    mov [esp + 14*4], eax

    ; Set TSS ESP0
    mov ebx, [ebp + 16]
    mov edi, 0x1004
    mov [edi], ebx

    ; Set Page directory
    mov edx, [ebp + 12]
    mov eax, [edx]
    test eax, eax
    jnz .d1
    call mmu_newdir
    mov edx, [ebp + 12]
    mov [edx], eax

    ; Init kernel stack
    mov ebx, [ebp + 16]
    mov cr3, eax
    mov byte [ebx], 0
    jmp .d2
  .d1:
    mov cr3, eax
  .d2:

    ; call kswitchdump

  ; End of interupt
    mov al,0x20
    out 0x20,al

  ; Load register
    pop gs
    pop fs
    pop es
    pop ds
    popad

  ; Jump
    iret



; =============================================
; =============================================

;extern kval
;extern kswitchdump

kCpu_Switch2:

    cli
    push ebp
    mov ebp, esp
    mov esp, 0x6800

  ; Copy REGS on stack
    mov ax, ds
    mov es, ax
    sub esp, SIZEOF_CPU_REGS - 8
    mov ecx, SIZEOF_CPU_REGS - 8
    mov esi, [ebp + 8]
    mov edi, esp
    rep movsb

  ; Fix EFLAGS
    mov eax, [esp + 14*4]
    or eax, 0x200
    and eax, 0xffffbfff
    mov [esp + 14*4], eax

  ; Set TSS ESP0
    mov ebx, [ebp + 16]
    mov edi, 0x1004
    mov [edi], ebx

  ; Set Page directory
    mov eax, [ebp + 12]
    mov cr3, eax

    ; call kswitchdump

  ; End of interupt
    mov al,0x20
    out 0x20,al


  ; Load register
    pop gs
    pop fs
    pop es
    pop ds
    popad

    mov eax, [esp - 20 ]
    mov esp, eax

  ; Jump
    iret


    mov esi, [ebp + 8]
    push esi
    ;call kval
    jmp $

    ; call kregisters





; cpu_hlt ----------------------------------

cpu_halt_:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov edi, [ebp + 12]
    sub eax, 10
    cli
    mov esp, eax

    mov dword [esp + 0], cpu_halt_.pause    ; eip
    mov dword [esp + 4], 0x8              ; cs
    mov dword [esp + 8], 0x200            ; eflags

  ; Set TSS ESP0
    mov ebx, eax
    mov [edi], ebx   

    mov ax, SGMT_KRN_DATA
    mov ds, ax
    mov es, ax

  ; End of interupt
    mov al,0x20
    out 0x20,al

  ; Jump
    iret

  .pause:
    sti
    hlt
    jmp .pause

    
; cpuid ------------------------------------

cpuid:
    push ebp
    mov ebp, esp
    push ecx
    push edx
    mov eax, [ebp + 8]
    mov ecx, [ebp + 12]
    cpuid
    mov edi, [ebp + 16]
    mov [edi], eax
    mov [edi + 4], ebx
    mov [edi + 8], edx
    mov [edi + 12], ecx
    pop edx
    pop ecx
    leave
    ret




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

extern kpanic, sys_ex, sys_irq, page_fault


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




IntEx00_Handler:
    push dword 0x00
    jmp IntEx_Handler
IntEx01_Handler:
    push dword 0x01
    jmp IntEx_Handler
IntEx02_Handler:
    push dword 0x02
    jmp IntEx_Handler
IntEx03_Handler:
    push dword 0x03
    jmp IntEx_Handler
IntEx04_Handler:
    push dword 0x04
    jmp IntEx_Handler
IntEx05_Handler:
    push dword 0x05
    jmp IntEx_Handler
IntEx06_Handler:
    push dword 0x06
    jmp IntEx_Handler
IntEx07_Handler:
    push dword 0x07
    jmp IntEx_Handler
IntEx08_Handler:
    push dword 0x08
    jmp IntEx_Handler
IntEx09_Handler:
    push dword 0x09
    jmp IntEx_Handler
IntEx0A_Handler:
    push dword 0x0a
    jmp IntEx_Handler
IntEx0B_Handler:
    push dword 0x0b
    jmp IntEx_Handler
IntEx0C_Handler:
    push dword 0x0c
    jmp IntEx_Handler
IntEx0D_Handler:
    push dword 0x0d
    jmp IntEx_Handler
IntEx0E_Handler: ; Page fault
    SAVE_REGS
    push esp
    push dword [esp + 52] ; get error code
    mov eax, cr2
    push eax
    call page_fault
    add esp, 12
    LOAD_REGS
    add esp, 4
    iret

IntEx0F_Handler:
    push dword 0x0f
    jmp IntEx_Handler
IntEx_Handler:
    call sys_ex
    iret

IRQ0_Handler:
    push dword 0
    jmp IRQ_Handler
IRQ1_Handler:
IRQ2_Handler:
IRQ3_Handler:
IRQ4_Handler:
IRQ5_Handler:
IRQ6_Handler:
IRQ7_Handler:
IRQ8_Handler:
IRQ9_Handler:
IRQ10_Handler:
IRQ11_Handler:
IRQ12_Handler:
IRQ13_Handler:
IRQ14_Handler:
IRQ15_Handler:
    push dword 15
IRQ_Handler:
    call sys_irq
    iret

SysCall_Handler:
Interrupt_Handler:
    push .msg
    call kpanic
    ret

  .msg:
    db "EXP__", 10, 0







; ---------------------------------
; ---------------------------------
global outb, outw, inb, inw
global insl, insw, outsw

%define Stck_Port   8
%define Stck_Value  12
%define Stck_Buffer 12
%define Stck_Length 16
; ---------------------------------
outb:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        mov al, [esp + Stck_Value]
        out dx, al
        leave
        ret

; ---------------------------------
outw:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        mov ax, [esp + Stck_Value]
        out dx, ax
        leave
        ret

; ---------------------------------
inb:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        xor eax, eax
        in al, dx
        leave
        ret

; ---------------------------------
inw:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        xor eax, eax
        in ax, dx
        leave
        ret

; ---------------------------------
; void insl (uint16_t port, void* addr, uint32_t count);
insl:
    push ebp
    mov ebp, esp
    mov dx, [esp + Stck_Port]
    mov edi, [esp + Stck_Buffer]
    mov ecx, [esp + Stck_Length]
  .loop:
    in eax, dx
    mov [edi], eax
    add edi, 4
    loop .loop
    leave
    ret


; ---------------------------------
insw: ;(p,b,l)
    push ebp
    mov ebp, esp
    mov dx, [esp + Stck_Port]
    mov edi, [esp + Stck_Buffer]
    mov ecx, [esp + Stck_Length]
  .loop:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .loop
    leave
    ret

; ---------------------------------
outsw: ;(p,b,l)
    push ebp
    mov ebp, esp
    mov dx, [esp + Stck_Port]
    mov edi, [esp + Stck_Buffer]
    mov ecx, [esp + Stck_Length]
  .loop:
    mov ax, [edi]
    out dx, ax
    add edi, 2
    loop .loop
    leave
    ret




