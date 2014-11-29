use32
; [org 0x20000]

%include "arch/i386/kern/config.asm"
%include "arch/i386/kern/grub.asm"

global _start

extern kCpu_Initialize, kCore_Initialize
extern init_pic

; GRUB symbols -----------------------------
extern kGrub_Initialize

_start:
; MultiBootHeader --------------------------
    dd GRUB_MAGIC1
    dd GRUB_FLAGS
    dd - GRUB_FLAGS - GRUB_MAGIC1

    dd _start       ; header_addr
    dd _start       ; load_addr
    dd 0            ; load_end_addr
    dd 0
    dd start

    dd 0            ; mode
    dd 800           ; width
    dd 600           ; height
    dd 0            ; depth

krpInfo:
    dd 0, 0 ; Offset, length

align 16
start:
; Start ------------------------------------
    cli
    mov esp, kMem_kstackptr - 0x10
    cmp eax, GRUB_MAGIC2
    je .grubLoad

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
    call kGrub_Initialize
    pop ebx
    test eax, eax
    jnz .errorLoader

    mov byte [0xB8000], 'G'
    mov byte [0xB8001], 0x57
    mov byte [0xB8002], 'o'
    mov byte [0xB8003], 0x57
    mov byte [0xB8004], 0x00


startup:
; Initialize CPU ---------------------------
    xor eax, eax
    mov edi, eax
    mov ecx, 2048
    rep stosd

    call kCpu_Initialize

    mov esp, kMem_kstackptr - 0x10
    lgdt [.gdtregs]
    jmp _x86_kCode_Sgmt:.reloadCS
  .reloadCS:
    mov ax, _x86_kData_Sgmt
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov byte [0xB8004], '.'
    mov byte [0xB8005], 0x57

    call kcpu_pic  ; init_pic
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

    ; Initialize timer
    ; mov ebx, 0x220 ; Frequency in Hz
    ; call PIT_initialize

    call kCore_Initialize
    sti
    jmp $


; =============================================
; =============================================

; Data -------------------------------------
align 4
  .gdtregs:
    dw 0xF8, 0, 0, 0
  .idtregs:
    dw 0x7F8, 0x800, 0, 0



; ------------------------------------------
global krpPack, krpLength
;align 256
krpPack:
;incbin "krp.tar"
krpPackEnd:

align 8
krpLength:
    dd krpPackEnd - krpPack

; ------------------------------------------

%include "arch/i386/kern/int.asm"
%include "arch/i386/kern/io.asm"
%include "arch/i386/kern/pit.asm"



; i386 routines ----------------------------

kcpu_pic:
    mov al, 0x11  ; initialisation de ICW1
    out 0x20, al
    jmp .1        ; temporisation
    .1:
    mov al, 0x20  ; initialisation de ICW2
    out 0x21, al  ; vecteur de depart = 32
    jmp .2        ; temporisation
    .2:
    mov al, 0x04  ; initialisation de ICW3
    out 0x21, al
    jmp .3
    .3:
    mov al, 0x01  ; initialisation de ICW4
    out 0x21, al
    jmp .4
    .4:
    xor al, al  ; Mask
    out 0x21, al


    mov al, 0x11  ; initialisation de ICW1
    out 0xA0, al
    jmp .5        ; temporisation
    .5:
    mov al, 0x70  ; initialisation de ICW2
    out 0xA1, al  ; vecteur de depart = 32
    jmp .6        ; temporisation
    .6:
    mov al, 0x02  ; initialisation de ICW3
    out 0xA1, al
    jmp .7
    .7:
    mov al, 0x01  ; initialisation de ICW4
    out 0xA1, al
    jmp .8
    .8:
    xor al, al  ; Mask
    out 0x21, al

    ret


; =============================================
; =============================================



%define SIZEOF_CPU_REGS 17*4
; =============================================
global kCpu_SwitchContext
extern kregisters, kTty_HexDump
; void kCpu_SwitchContext (kCpuRegs_t* regs, uint32_t dir)

; void kCpu_Switch (kCpuRegs_t* regs, uint32_t* dir, uint32_t kstack);
global kCpu_Switch, kCpu_Switch2
extern kpg_new, kTty_HexChar, kDBG


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
    call kpg_new
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

extern kval
extern kswitchdump

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

    call kswitchdump

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
    call kval
    jmp $

    call kregisters




; =============================================
; =============================================


kcpuhlt:

    cli
    mov esp, 0x6800
    mov dword [esp + 16], 0x18        ; ss
    mov dword [esp + 12], 0x6f00      ; esp
    mov dword [esp + 8], 0x200        ; eflags
    mov dword [esp + 4], 0x8          ; cs
    mov dword [esp], kcpuhlt.go       ; eip

  ; End of interupt
    mov al,0x20
    out 0x20,al

  ; Jump
    iret

  .go:
    hlt
    jmp $


; =============================================
; =============================================

global _geterrno
errno: dd 0
_geterrno:
    mov eax, errno
    ret
