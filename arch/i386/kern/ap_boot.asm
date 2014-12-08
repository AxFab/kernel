
global ap_start
global ap_count
extern kprintf

ap_start:

;    mov eax, 0xf0000000



;  .pause1:
;    cli
;    hlt
;    jmp .pause1


    cli

  .spinlock:
    mov eax, 1
    lock xchg eax, [.lock]
    test eax, eax
    jnz .spinlock


    lgdt [startup.gdtregs]
    mov esp, kMem_kstackptr - 0x10 + 0x800
    jmp _x86_kCode_Sgmt:.reloadCS
  .reloadCS:
    mov ax, _x86_kData_Sgmt
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lidt [startup.idtregs]
    mov ax, _x86_TSS_Sgmt
    ltr ax

    mov eax, 0x2000 ; PAGE kernel
    mov cr3, eax
    mov eax, cr0
    or eax, (1 << 31) ; CR0 31b to activate mmu
    mov cr0, eax

    lock inc dword [ap_count]

    push dword .msg
    call kprintf

    xor eax, eax
    mov [.lock], eax

  .pause:
    cli
    hlt
    jmp .pause

  .msg:
    db "-- NEW CPU", 10, 0

  .lock:
    dd 0

ap_count:
    dd 0


%define    ICR_LOW          0x300
%define    SVR              0xF0
%define    APIC_ID          0x20
%define    LVT3             0x370
%define    APIC_ENABLED     0x100

%define    ACPI             0x100000


global cpu_svr
extern __step
cpu_svr:
    push ebp
    mov ebp, esp

    ; Enables the local APIC by setting bit 8 of the APIC spurious vector register
    mov esi, ACPI + SVR       ; Address of SVR
    mov eax, [esi]
    or eax, 0x100     ; APIC_ENABLED - Set bit 8 to enable (0 on reset)
    mov [esi], eax

    call __step

    mov esi, 0x0100370       ; Address of LVT3
    mov eax, [esi]
    and eax, 0xFFFFFF00       ; Clear out previous vector.
    or eax, 0x22 ;(acpi_err >> 12)     ; is the 8-bit vector the APIC error handler.
    mov [esi], eax

    call __step
    
    mov esi, 0x00100300  ; Load address of ICR low dword into ESI.
    mov eax, 0x000C4500  ; Load ICR encoding for broadcast INIT IPI
    
    ; to all APs into EAX.
    mov [esi], eax        ; Broadcast INIT IPI to all APs

    ; 10-millisecond delay loop.
    call __delay


    mov eax, 0x0C4600 | 0x21 ; (ap_start / 4096)     ; Load ICR encoding for broadcast SIPI IP
                                            ; to all APs into EAX, where xx is the vector computed in step 10.
    mov [esi], eax        ; Broadcast SIPI IPI to all APs
    ; 200-microsecond delay loop
    call __delay

    mov [esi], eax        ; Broadcast second SIPI IPI to all APs
    ; 200-microsecond delay loop
    call __delay

    
    ; Load ICR encoding from broadcast SIPI IP to all APs into EAX where xx is the vector computed in step 8.
    mov eax, 0x0C4600 | 0x21 ; (ap_start / 4096)      

    cli
    hlt
    jmp $



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




