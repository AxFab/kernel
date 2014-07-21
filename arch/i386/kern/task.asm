; =============================================================================
;        LICENSE FOR NON-COMMERCIAL USE ONLY - Simplified BSD
;
;  Copyright (c) 2012, Fabien Bavent
;  All rights reserved.
;
;  Redistribution and use in source and binary forms, with or without
;  modification, are permitted provided that the following conditions are met:
;
;    * Redistributions of source code must retain the above copyright notice,
;      this list of conditions and the following disclaimer.
;    * Redistributions in binary form must reproduce the above copyright notice,
;      this list of conditions and the following disclaimer in the documentation
;      and/or other materials provided with the distribution.
;    * Any redistribution, use, or modification is done solely for personal
;      benefit and not for any commercial purpose or for monetary gain.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
;  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;  POSSIBILITY OF SUCH DAMAGE.
; =============================================================================
use32

global krnThread_Launch, krnThread_Save, krnThread_Reset

; ------------------------------------------------------------------------------------------------
krnThread_Launch:
		mov esi, [esp + 8] ; get pointer on regs
		mov eax, [esp + 12] ; get directory of task
		sub esp, CpuSize / 4
		mov cr3, eax
		mov edi, esp
		mov ecx, CpuSize / 4
		rep movsd
		
		mov ebx, [esp + Regs_Flags]
		or ebx, 0x200
		and ebx, 0xFFFFBFFF
		mov [esp + Regs_Flags], ebx
		
		mov al, 0x20
		out 0x20, al
		LOAD_REGS
		sti
		iret
		
; ------------------------------------------------------------------------------------------------
krnThread_Save:
		push edi
		push esi
		mov edi, [ebp + 8] ; get pointer on regs store
		mov esi, [ebp + 8] ; get pointer on regs actual
		mov ecx, CpuSize / 4
		rep movsd
		pop esi
		pop edi
		ret

; ------------------------------------------------------------------------------------------------
krnThread_Reset:
		ret

