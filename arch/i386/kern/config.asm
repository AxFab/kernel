use32

;%define _KERR__UNKNOWN_BOOTLOADER_	2
;%define _KERR__BOOTLOADER_FAILURE_	3

;%define MeMap_KernelStackUp	0x20000 - 0x10
;%define MeMap_KernelStackUp	0x400000
;%define MeMap_UserCode 0x800000

;%define Sgmt_KernelCode 0x08
;%define Sgmt_KernelData 0x10
;%define Sgmt_KernelStack 0x18
;%define Sgmt_UserSpCode 0x23
;%define Sgmt_UserSpData 0x2d
;%define Sgmt_UserSpStack 0x33
;%define Sgmt_TaskStack 0x38

;%define TssReg 0x80

;%define CpuSize 0x80
;%define Regs_Flags 0x14




; New --------------------------------------

; Memory mapping




%define kMem_kstackptr 0x7000



; i386 GDT constante -----------------------
%define _x86_kCode_Sgmt     0x08
%define _x86_kData_Sgmt     0x10
%define _x86_kStack_Sgmt    0x18
%define _x86_uCode_Sgmt     0x23
%define _x86_uData_Sgmt     0x2d
%define _x86_uStack_Sgmt    0x33
%define _x86_TSS_Sgmt       0x38




%define Pic1Cmd 	0x20
%define Pic2Cmd 	0xA0
%define Pic1Data 	0x21
%define Pic2Data 	0XA1
%define Pic1Offset 	0x20
%define Pic2Offset 	0x28
%define PicInit 	0x11
%define Pic8086 	0x01


