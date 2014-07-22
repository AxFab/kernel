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
  .wait:
    xor eax, eax
    mov ebx, 0xb
    jmp .wait

; Data -------------------------------------
align 4
  .gdtregs:
    dw 0xF8, 0, 0, 0
  .idtregs:
    dw 0x7F8, 0x800, 0, 0



; ------------------------------------------

global krpPack, krpLength

align 256
krpPack:
incbin "krp.tar"
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


;; kCpu_Context (0x80000000);
global kCpu_Context
extern kTty_HexDump

kCpu_Context:

  ; Set Page directory 
    ; mov dword [0x8000000f], 0x0000feeb
    mov esp, 0x7000 - 0x10

  ; Set user-stack
    push dword 0x33 ; ss
    push dword 0xCFFFFFF0 ; esp
    pushf

  ; Fix flags
    pop eax
    or eax, 0x200
    and eax, 0xFFFFBFFF
    push eax

  ; Set code and data registers
    push 0x23
    push 0x80000000
    mov ax, 0x2B
    mov ds, ax

;    push dword 0x50
;    push esp
;    call kTty_HexDump
;    pop eax
;    pop eax

%define SIZEOF_CPU_REGS 17*4
; =============================================
global kCpu_SwitchContext
extern kCpu_DisplayRegs, kTty_HexDump
; void kCpu_SwitchContext (kCpuRegs_t* regs, uint32_t dir) 

; void kCpu_Switch (kCpuRegs_t* regs, uint32_t* dir);
global kCpu_Switch
extern kPg_NewDir, kTty_HexChar
kCpu_Switch:
kCpu_SwitchContext: 
    
    push ebp
    mov ebp, esp
    cli

  ; Set Page directory
    mov edx, [ebp + 12]
    mov eax, [edx]
    test eax, eax
    jnz .do
    call kPg_NewDir
    mov [edx], eax

  .do:
    mov cr3, eax

  ; Copy REGS on stack
    mov ax, ds
    mov es, ax
    sub esp, SIZEOF_CPU_REGS
    mov ecx, SIZEOF_CPU_REGS
    mov esi, [ebp + 8]
    mov edi, esp
    rep movsb

    add esp, 0x10
    mov [esp + 3*4], esp
    sub esp, 0x10
    
    mov eax, [esp + 14*4]
    or eax, 0x200
    and eax, 0xffffbfff
    mov [esp + 14*4], eax

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


 ; do switch
    mov al, 0x20
    out 0x20, al
    sti
    iret

    jmp $


global _geterrno
errno: dd 0
_geterrno:
    mov eax, errno
    ret
