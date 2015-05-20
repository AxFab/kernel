# Compile assembly for x86 kernel
# ---------------------------------------------------------------------------
ifeq ($(obj_dir),)
$(error  This file is part of a larger script!)
else

KDEPS += $(obj_dir)/crtk.o

$(obj_dir)/crtk.o: $(src_dir)/src/_x86/crt/crtk.asm
	$(Q) "    ASM "$<
	$(V) nasm -f elf32 -o $@ $^

# CD-rom
cdrom: OsCore.iso
	@ ls -lh $<

OsCore.iso: $(krn_img)
	@ mkdir -p iso/bin iso/boot/grub
	@ cp $(krn_img) iso/boot/kImage
	@ cp $(src_dir)/src/_x86/grub.cfg iso/boot/grub/grub.cfg
#	@ cp ?? bin/master
# Extract utilities package on iso !
	$(Q) "    ISO "$@
	$(V) grub-mkrescue -o $@ iso 2>/dev/null >/dev/null
	@ rm -rf iso


endif
