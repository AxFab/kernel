use32

%define SIZEOF_CPU_REGS 5

%define GRUB_MAGIC1     0x1BADB002
%define GRUB_MAGIC2     0x2BADB002
%define GRUB_FLAGS      0x00010007 ;0x00010003; 0x00010007

extern code, bss, end

%define MEM_KSTACK_PTR  0x7000

%define SGMT_KRN_CODE     0x08
%define SGMT_KRN_DATA     0x10
%define SGMT_KRN_STACK    0x18
%define SGMT_USR_CODE     0x23
%define SGMT_USR_DATA     0x2d
%define SGMT_USR_STACK    0x33
%define _x86_TSS_Sgmt     0x38


global _start
global cpuid
global cpu_halt_

extern grub_initialize
extern kernel_start, kernel_ready
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

    dd 0            ; mode
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
    mov byte [0xB8004], 0x00
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



; @todo - Change TSS, CR3*, Entry*, Params*
;          * Only if needed

; void cpu_restart_(cr3, kstk, entry, param, ustack, tssAdd)
global cpu_restart_, cpu_resume_, cpu_wait
cpu_restart_:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8] ; Cr3
    mov ebx, [ebp + 12] ; Tss.esp
    mov ecx, [ebp + 16] ; Entry
    mov edx, [ebp + 20] ; Param
    mov esi, [ebp + 24] ; UserStack
    mov edi, [ebp + 28] ; TSS Address

  ; Set TSS ESP0
    add edi, 4
    mov [edi], ebx

  ; Set Page directory
    mov esp, MEM_KSTACK_PTR - 64 - 16
    mov cr3, eax
    mov byte [ebx], 0
    ; mov byte [esi], 0

  ; Create Stack
  ; Rewrite CS SS EIP EAX ESP EFLAGS
    xor ebx, ebx
    mov bx, SGMT_USR_DATA
    mov [esp + 0] , bx  ; gs
    mov [esp + 4] , bx  ; fs
    mov [esp + 8] , bx  ; es
    mov [esp + 12], bx  ; ds
    xor ebx, ebx
    mov [esp + 16], ebx ; edi
    mov [esp + 20], ebx ; esi
    mov [esp + 24], ebx ; ebp
    mov [esp + 28], ebx ; esp
    mov [esp + 32], ebx ; ebx
    mov [esp + 36], ebx ; edx
    mov [esp + 40], ebx ; ecx
    mov [esp + 44], edx ; eax
    mov [esp + 48], ecx ; eip
    xor ebx, ebx
    mov bx, SGMT_USR_CODE
    mov [esp + 52], bx  ; cs
    mov ebx, 0x00000200
    mov [esp + 56], ebx  ; eflags
    mov [esp + 60], esi  ; esp
    xor ebx, ebx
    mov bx, SGMT_USR_STACK
    mov [esp + 64], ebx  ; ss

  ; End of interupt (IN CASE)
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret


; cpu_start ----------------------------------
; void cpu_resume_(cr3, kstk, sp, tssAdd)
cpu_resume_:
cpu_start_:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8] ; Cr3
    mov ebx, [ebp + 12] ; Tss.esp
    mov ecx, [ebp + 16] ; SP
    mov edi, [ebp + 20] ; TSS Address

  ; Set TSS ESP0
    add edi, 4
    mov [edi], ebx

  ; Set Page directory
    mov cr3, eax
    mov byte [ebx], 0
    ; mov byte [esi], 0

  ; Create Stack
  ; Rewrite CS SS EIP EAX ESP EFLAGS
    mov esp, ecx

  ; End of interupt (IN CASE)
    mov al,0x20
    out 0x20,al
    LOAD_REGS
    iret


; cpu_hlt ----------------------------------

cpu_halt_:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8] ; CPU Stack
    mov edi, [ebp + 12] ; TSS Address
    sub eax, 10
    cli
    mov esp, eax

    mov dword [esp + 0], cpu_halt_.pause  ; eip
    mov dword [esp + 4], 0x8              ; cs
    mov dword [esp + 8], 0x200            ; eflags

  ; Set TSS ESP0
    add edi, 4
    mov [edi], eax

    mov ax, SGMT_KRN_DATA
    mov ds, ax
    mov es, ax

  ; End of interupt (IN CASE)
    mov al,0x20
    out 0x20,al
    iret

  .pause:
    sti
    hlt
    jmp .pause

; void cpu_wait()
cpu_wait:
    int 0x31
    ret


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
global SysWait_Handler
global Interrupt_Handler

extern kpanic, sys_ex, sys_irq, sys_call, 
extern general_protection, page_fault



IntEx00_Handler:
    SAVE_REGS
    push dword 0x00
    jmp IntEx_Handler
IntEx01_Handler:
    SAVE_REGS
    push dword 0x01
    jmp IntEx_Handler
IntEx02_Handler:
    SAVE_REGS
    push dword 0x02
    jmp IntEx_Handler
IntEx03_Handler:
    SAVE_REGS
    push dword 0x03
    jmp IntEx_Handler
IntEx04_Handler:
    SAVE_REGS
    push dword 0x04
    jmp IntEx_Handler
IntEx05_Handler:
    SAVE_REGS
    push dword 0x05
    jmp IntEx_Handler
IntEx06_Handler:
    SAVE_REGS
    push dword 0x06
    jmp IntEx_Handler
IntEx07_Handler:
    SAVE_REGS
    push dword 0x07
    jmp IntEx_Handler
IntEx08_Handler:
    SAVE_REGS
    push dword 0x08
    jmp IntEx_Handler
IntEx09_Handler:
    SAVE_REGS
    push dword 0x09
    jmp IntEx_Handler
IntEx0A_Handler:
    SAVE_REGS
    push dword 0x0a
    jmp IntEx_Handler
IntEx0B_Handler:
    SAVE_REGS
    push dword 0x0b
    jmp IntEx_Handler
IntEx0C_Handler:
    SAVE_REGS
    push dword 0x0c
    jmp IntEx_Handler

IntEx0D_Handler:
    SAVE_REGS
    push esp
    push dword [esp + 52] ; get error code
    call general_protection
    add esp, 8
    LOAD_REGS
    add esp, 4
    iret

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

; ================================

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

; ================================
SysCall_Handler:
    SAVE_REGS
    push esp
    call sys_call
    add esp, 4
    LOAD_REGS
    iret

extern sys_wait_
SysWait_Handler:
    cli
    SAVE_REGS
    push esp

;    mov byte [0xB8010], '.'
;    mov byte [0xB8015], 0x57
    call sys_wait_
    ; sys_wait_ should never return
    mov byte [0xB8010], 'F'
    mov byte [0xB8011], 0x57
    add esp, 4
    LOAD_REGS
    iret
    

Interrupt_Handler:
  IRQ_HANDLER -1


    push .msg
    call kpanic
    ret

  .msg:
    db "EXP__", 10, 0







; ---------------------------------
; ---------------------------------
global outb, outw, outl, inb, inw, inl
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
outl:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        mov eax, [esp + Stck_Value]
        out dx, eax
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
inl:
        push ebp
        mov ebp, esp
        mov dx, [esp + Stck_Port]
        xor eax, eax
        in eax, dx
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






; ==========================================
; CR0 Register
;   PE [0] - Real-address mode:0
;   MP [1] - WAIT/FWAIT instruction not trapped:0
;   EM [2] - x87 FPU instruction not trapped:0
;   TS [3] - No task switch:0
;   NE [5] - External x87 FPU error reporting:0
;   WP [16] - Write-protect disabled:0
;   AM [18] - Alignement check disabled:0
;   NW [29] - Not write-through disabled:1
;   CD [30] - Caching disabled:1
;   PG [31] - Paging disabled:0
; ------------------------------------------
global x86_ActiveFPU
global x86_ActiveCache


x86_ActiveFPU:
    mov eax, cr0
    and eax, 0xfffffffb
    or eax, 2
    mov cr0, eax
    ret


x86_ActiveCache:
    mov eax, cr0
    and eax, 0x9fffffff
    mov cr0, eax
    ret


global UM_TEST
UM_TEST:
    jmp $


%define  CPU_GDT    0x700
%define  CPU_LOCK   0x7f0
%define  CPU_COUNT  0x7f8

; ==========================================
global x86_ApStart 
align 4096
x86_ApStart:
use16
    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax

    ; Activate GDT
    mov di, CPU_GDT
    mov word [di], 0xF8
    lgdt [di]

    ; Mode protected
    mov eax, cr0
    or  ax, 1
    mov cr0, eax        ; PE set to 1 (CR0)

    lock inc word [CPU_COUNT]

    jmp .next
  .next:

    mov ax, 0x10                  ; data segment
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov es, ax
    mov ss, ax

    jmp dword 0x8:ap_32start    ; code segment


; ------------------------------------------
align 128
use32

; ------------------------------------------
extern mmu_newpage, mmu_resolve
; extern kprintf

ap_32start:
    cli

    lidt [startup.idtregs]

    ; mov ax, _x86_TSS_Sgmt
    ; ltr ax

    mov eax, 0x2000 ; PAGE kernel
    mov cr3, eax
    mov eax, cr0
    or eax, (1 << 31) ; CR0 31b to activate mmu
    mov cr0, eax

    ; Lock CPU Initialization Spin-Lock
  .spinlock:
    cmp dword [CPU_LOCK], 0    ; Check if lock is free
    je .getlock
    pause
    jmp .spinlock
  .getlock:
    mov eax, 1
    xchg eax, [CPU_LOCK]  ; Try to get lock
    cmp eax, 0            ; Test is successful
    jne .spinlock
  .criticalsection:

    ; Create stack
    ; @todo Lock up to max CPU (Stack: 1024 CPUs, GDT: 120, TSS:8)
    mov eax, [.stack]
    add eax, 0x1000
    mov [.stack], eax
    add eax, 0x1000 - 0x10
    mov esp, eax

    ; @todo fillout TSS

    ;mov [esp], eax                   ; Address of mapping
    ;push dword [esp + 8], 0x10006     ; WMA_WRITE | WMA_KERNEL
    ;push dword [esp + 12], 1          ; reset to zero
    ;call mmu_newpage
    ;mov [esp + 4], eax
    ;call mmu_resolve
    ; mov esp, [esp]
    ; add esp, 0x1000 - 0x10

    ; Print a message
    ; push esp
    ; push .msg 
    ; call kprintf

    ; Unlock the spinlock
    xor eax, eax
    mov [CPU_LOCK], eax

    call kernel_ready

  .pause:
    sti
    hlt
    jmp .pause

  .msg:
    db "Stack 0x%08x...", 10, 0


  .stack:
    dd 0x100000



global x86_ApError
align 4096
x86_ApError:
    nop
    jmp $



