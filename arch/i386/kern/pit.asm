

; =============================================================================
;                 I/O port       Usage
%define PIT_CH0   0x40         ; Channel 0 data port (read/write)
%define PIT_CH1   0x41         ; Channel 1 data port (read/write)
%define PIT_CH2   0x42         ; Channel 2 data port (read/write)
%define PIT_CMD   0x43         ; Mode/Command register (write only, a read is ignored)


; =============================================================================
PIT_data:
  .sys_fractions:    dd 0   ; Fractions of 1 mS since initialized
  .sys_mS:           dd 0   ; Number of whole mS since initialized (50days)
  .fractions:        dd 0   ; Fractions of 1 mS between IRQs
  .mS:               dd 0   ; Number of whole mS between IRQs
  .frequency:        dd 0   ; Actual frequency of PIT
  .reload:           dw 0   ; Current PIT reload value


; =============================================================================
; Input
; ebx   Desired PIT frequency in Hz
PIT_initialize:
  .start:
    pushad
 
    ; Bounding frequency value [ 1 - 65536 ]
    mov eax, 0x10000          ; eax = slowest possible frequency (65536)
    cmp ebx, 18
    jbe .gotReloadValue 
    mov eax, 1                ; ax = fastest possible frequency (1)
    cmp ebx, 1193181     
    jae .gotReloadValue 

    ; Calculate the reload value
    mov eax, 3579545          ; eax = 3579545

    div ebx                   ; eax = 3579545 / frequency, edx = remainder
    cmp edx, 3579545 / 2      ; Is the remainder more than half?
    jb .l1                    ; no, round down
    inc eax                   ; yes, round up
  .l1:

    mov ebx, 3
    div ebx                   ; eax = (3579545 * 256 / 3 * 256) / frequency
    cmp edx, 3 / 2            ; Is the remainder more than half?
    jb .l2                    ; no, round down
    inc eax                   ; yes, round up
  .l2:
 
 
    ; Store the reload value and calculate the actual frequency
 
  .gotReloadValue:
    push eax                  ; Store reload_value for later
    mov [PIT_data.reload], ax ; Store the reload value for later
    mov ebx,eax               ; ebx = reload value
 
    mov eax, 3579545          ; eax = 3579545
    div ebx                   ; eax = 3579545 / reload, edx = remainder
    cmp edx, 3579545 / 2      ; Is the remainder more than half?
    jb .l3                    ; no, round down
    inc eax                   ; yes, round up
 .l3:

    mov ebx,3                 ; eax = 3579545 / reload_value
    div ebx                   ; eax = (3579545 / 3) / frequency
    cmp edx, 3 / 4            ; Is the remainder more than half?
    jb .l2                    ; no, round down
    inc eax                   ; yes, round up
 .l4:

    mov [PIT_data.frequency], eax      ; Store the actual frequency
 
 
 ; Calculate the amount of time between IRQs in 32.32 fixed point
 ;
 ; Note: The basic formula is:
 ;           time in ms = reload_value / (3579545 / 3) * 1000
 ;       This can be rearranged in the follow way:
 ;           time in ms = reload_value * 3000 / 3579545
 ;           time in ms = reload_value * 3000 / 3579545 * (2^42)/(2^42)
 ;           time in ms = reload_value * 3000 * (2^42) / 3579545 / (2^42)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^42) * (2^32)
 ;           time in ms * 2^32 = reload_value * 3000 * (2^42) / 3579545 / (2^10)
 
    pop ebx                           ;ebx = reload_value
    mov eax,0xDBB3A062                ;eax = 3000 * (2^42) / 3579545
    mul ebx                           ;edx:eax = reload_value * 3000 * (2^42) / 3579545
    shrd eax,edx,10
    shr edx,10                        ;edx:eax = reload_value * 3000 * (2^42) / 3579545 / (2^10)
 
    mov [PIT_data.mS], edx                 ;Set whole mS between IRQs
    mov [PIT_data.fractions], eax          ;Set fractions of 1 mS between IRQs
 
 
    ; Program the PIT channel
    pushfd
    cli                       ; Disabled interrupts (just in case)
 
    mov al, 0x34              ; channel 0, lobyte/hibyte, rate generator
    out PIT_CMD, al
 
    mov ax, [PIT_data.reload] ; ax = 16 bit reload value
    out PIT_CH0, al           ; Set low byte of PIT reload value
    mov al, ah                ; ax = high 8 bits of reload value
    out PIT_CH0, al           ; Set high byte of PIT reload value
 
    popfd
    popad
    ret

; =============================================================================
PIT_read_reloadcount:
  pushfd
  cli
  mov al, 00000000b    ; al = channel in bits 6 and 7, remaining bits clear
  out 0x43, al         ; Send the latch command
 
  in al, 0x40          ; al = low byte of count
  mov ah, al           ; ah = low byte of count
  in al, 0x40          ; al = high byte of count
  rol ax, 8            ; al = low byte, ah = high byte (ax = current count)
  popfd
  ret

; =============================================================================
PIT_set_reloadcount:
  pushfd
  cli
  out 0x40, al        ; Set low byte of reload value
  rol ax, 8           ; al = high byte, ah = low byte
  out 0x40, al        ; Set high byte of reload value
  rol ax, 8           ; al = low byte, ah = high byte (ax = original reload value)
  popfd
  ret

; =============================================================================
;global IRQ0_handler
;extern kInt_Clock, PIT_Period, kSYS
; ---

;IRQ0_handler:
;  .start:
;    SAVE_REGS
;   
;    xor edx, edx
;    mov eax, [PIT_Period]
;    add [kSYS], eax            ; Update system timer low-bytes
;    adc [kSYS + 4], edx        ; Update system timer high-bytes
;   
;    call kInt_Clock
;   
;    mov al,0x20
;    out 0x20,al
;    LOAD_REGS
;    iret

; =============================================================================



