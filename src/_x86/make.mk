
SRC_ka = $(wildcard src/_x86/*.c)

$(obj_dir)/crtk.o: $(src_dir)/_x86/crt/crtk.asm
	nasm -f elf32 -o $@ $^
