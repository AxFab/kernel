use32

global _start, start
global sname, exit, exec
global close, open, read, write
global _errno

extern main

global syscall_1A, syscall_2A, syscall_3A, syscall_4A, syscall_5A


start:
_start:
    sub esp, 16
    and esp, 0xfffffff0
    xor eax, eax
    mov [esp], eax
    mov [esp + 4], eax
    call main
    mov ecx, eax
    mov eax, 0x10
    int 0x30
    jmp $

_errno:
    dd 0

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
sname:
    mov eax, 0x00
    jmp os_syscall_2

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
exit:
    mov eax, 0x10
    jmp os_syscall_1

exec:
    mov eax, 0x11
    jmp os_syscall_2

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
close:
    mov eax, 0x20
    jmp os_syscall_1

open:
    mov eax, 0x21
    jmp os_syscall_3

read:
    xor eax, eax
    mov al, 0x22
    jmp os_syscall_3

write:
    mov eax, 0x23
    jmp os_syscall_3

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
syscall_1A:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 12]
    jmp os_syscall_1.push
os_syscall_1:
    push ebp
    mov ebp, esp
  .push:
    mov ecx, [ebp + 8]   ; arg1
    int 0x30
    leave
    ret

syscall_2A:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 16]
    jmp os_syscall_2.push
os_syscall_2:
    push ebp
    mov ebp, esp
  .push:
    mov ecx, [ebp + 8]   ; arg1
    mov edx, [ebp + 12]  ; arg2
    int 0x30
    leave
    ret

syscall_3A:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 20]
    jmp os_syscall_3.push
os_syscall_3:
    push ebp
    mov ebp, esp
    push ebx
  .push:
    mov ecx, [ebp + 8]   ; arg1
    mov edx, [ebp + 12]  ; arg2
    mov ebx, [ebp + 16]  ; arg3
    int 0x30
    pop ebx
    leave
    ret

syscall_4A:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 24]
    jmp os_syscall_4.push
os_syscall_4:
    push ebp
    mov ebp, esp
    push ebx
    push esi
  .push:
    mov ecx, [ebp + 8]   ; arg1
    mov edx, [ebp + 12]  ; arg2
    mov ebx, [ebp + 16]  ; arg3
    mov esi, [ebp + 20]  ; arg4
    int 0x30
    pop esi
    pop ebx
    leave
    ret

syscall_5A:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 28]
    jmp os_syscall_5.push
os_syscall_5:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
  .push:
    mov ecx, [ebp + 8]   ; arg1
    mov edx, [ebp + 12]  ; arg2
    mov ebx, [ebp + 16]  ; arg3
    mov esi, [ebp + 20]  ; arg4
    mov edi, [ebp + 24]  ; arg5
    int 0x30
    pop edi
    pop esi
    pop ebx
    leave
    ret

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
