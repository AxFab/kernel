use32
global outb, outw, inb, inw, insw, outsw
global insl
global cr3Swap, cr3Set

%define Stck_Port	8
%define Stck_Value	12
%define Stck_Buffer	12
%define Stck_Length	16
%define Stck_NewDir 8
; ------------------------------------------------------------------------------------------------
outb:
		push ebp
		mov ebp, esp
		mov dx, [esp + Stck_Port]
		mov al, [esp + Stck_Value]
		out dx, al
		leave
		ret

; ------------------------------------------------------------------------------------------------
outw:
		push ebp
		mov ebp, esp
		mov dx, [esp + Stck_Port]
		mov ax, [esp + Stck_Value]
		out dx, ax
		leave
		ret

; ------------------------------------------------------------------------------------------------
inb:
		push ebp
		mov ebp, esp
		mov dx, [esp + Stck_Port]
		xor eax, eax
		in al, dx
		leave
		ret

; ------------------------------------------------------------------------------------------------
inw:
		push ebp
		mov ebp, esp
		mov dx, [esp + Stck_Port]
		xor eax, eax
		in ax, dx
		leave
		ret

; ------------------------------------------------------------------------------------------------
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


; ------------------------------------------------------------------------------------------------
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

; ------------------------------------------------------------------------------------------------
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

; ------------------------------------------------------------------------------------------------
; Put the directory in cr3 and return previous register value
; _Table Cr3Swap (_Table newdir)
cr3Swap:
		push ebp
		mov ebp, esp
		mov ebx, [ebp + Stck_NewDir]
		mov eax, cr3
		mov cr3, ebx
		leave
		ret

; ------------------------------------------------------------------------------------------------
; void Cr3Set (_Table newdir)
cr3Set: ; Enable Paging
		push ebp
		mov ebp, esp
		mov eax, [ebp + Stck_NewDir]
		mov cr3, eax
		mov eax, cr4
		or eax, 0x00000010
		mov cr4, eax
		mov eax, cr0
		or eax, 0x80000000
		mov cr0, eax
		leave
		ret
