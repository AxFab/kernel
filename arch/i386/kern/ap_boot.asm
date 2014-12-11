

; ==========================================
extern kprintf
extern mmu_newpage
extern mmu_resolve
extern kernel_ready

; ------------------------------------------
; AP start-up
%define  CPU_GDT    0x700
%define  CPU_LOCK   0x7f0
%define  CPU_COUNT  0x7f8

; ==========================================
align 4096
use16

global ap_start
; ------------------------------------------
ap_start:
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

global ap_32start
; ------------------------------------------


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

    ; Parameters
    mov esp, 0x67e0
    mov eax, [.stack]
    add eax, 0x1000
    mov [.stack], eax
    mov [esp], eax                   ; Address of mapping
    mov dword [esp + 8], 0x10006     ; WMA_WRITE | WMA_KERNEL
    mov dword [esp + 12], 1          ; reset to zero

    call mmu_newpage
    mov [esp + 4], eax
    call mmu_resolve
    mov esp, [esp]
    add esp, 0x1000 - 0x10

    ; Print a message
    push esp
    push .msg 
    call kprintf

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




; ==========================================
align 4096

global acpi_err
; ------------------------------------------
acpi_err:
    push dword .msg
    call kprintf
    pop eax

    jmp acpi_err


  .msg:
    db "-- acpi error", 10, 0



sp_lock:
    mov edi, [esp + 8]
    mov eax, [edi]
    test eax, eax           ; Check if lock is free
    jz .get
    pause                   ; Short delay
    jmp sp_lock
  .get:
    mov eax, 1
    xchg eax, [edi]         ; Try to get lock
    cmp eax, eax            ; Test if successful
    jnz sp_lock
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

; ==========================================




