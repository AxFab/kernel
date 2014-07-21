use32

global _start, start, ossyscall, 
global read, write
extern main


start:
_start:
    sub esp, 16
    and esp, 0xfffffff0
    xor eax, eax
    mov [esp], eax
    mov [esp + 4], eax
    call main
    mov ebx, eax
    mov eax, 0x10
    int 0x30
    jmp $

ossyscall:
    push ebp
    mov ebp, esp
    ;pushad
    sub esp, 0x20
    mov eax, [ebp]
    mov ecx, [ebp + 4]
    mov edx, [ebp + 8]
    mov ebx, [ebp + 12]
    ;mov esi, [ebp + 16]
    ;mov edi, [ebp + 20]
    int 0x30
    ;mov [esp + 0], eax
    ;popad
    leave
    ret



; ----------------------------------------------------------------------------
; ssize_t read(int fd, void *buf, size_t count);
; ssize_t write(int fd, void *buf, size_t count);
read:
    xor eax, eax
    mov al, 0x22
    jmp xchdata

write:
    mov eax, 0x23
    jmp xchdata

xchdata:
    push ebp
    mov ebp, esp
    sub esp, 8
    pushad
    mov ecx, [ebp + 8]   ; fd
    mov edx, [ebp + 12]
    mov ebx, [ebp + 16]
    lea esi, [ebp - 4]   ; char_reads
    int 0x30
    ; mov errno, eax
    mov eax, [ebp - 4]
    popad
    leave
    ret

; ----------------------------------------------------------------------------



