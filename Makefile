# Makefile for SmokeOS kernel
# ----------------------------------------------------

# *** Settings ***
E = @ true
V = 
arch = i386

CC = gcc -c -o
CCD = gcc -MM -o
LD = ld -o
LDS = ld -shared -o
AR = ar rc
AS = nasm -f elf32 -o
LDK = ld --oformat=binary -Map $@.map -Ttext 20000 -o
LDX = ld -T scripts/smoke.ld -o

AXLIBC = ./3rdparty/axlibc

C_FLAGS = -nostdinc 
S_FLAGS =
L_FLAGS =
INC = -I $(ORIGIN_DIR)/include -I arch/i386/include/ -I $(AXLIBC)/include/ -I $(AXLIBC)/internal/
DEF = -D__EX -D__KERNEL -D__x86_64__
LIB =
LBK = $(AXLIBC)/lib/libAxRaw.a

# *** Folders ***
BUILD_DIR = .
ORIGIN_DIR = .
OBJS_DIR = $(BUILD_DIR)/obj
SOURCE_DIR = $(ORIGIN_DIR)/src
BIN_DIR = $(BUILD_DIR)/bin
BOOT_DIR = $(BUILD_DIR)/boot
LIB_DIR = $(BUILD_DIR)/lib


all: $(BOOT_DIR)/kImage
# 	@ echo 'No target'

# ----------------------------------------------------
# *** Common rules ***

$(OBJS_DIR)/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS) $(INC) $(DEF)

$(OBJS_DIR)/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $@ $< $(C_FLAGS) $(INC) $(DEF)

ifeq ($(arch),i386)
$(OBJS_DIR)/%.o: $(SOURCE_DIR)/%.asm
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile ASM source file: $@'
	$(V) $(AS) $@ $< $(S_FLAGS)

$(OBJS_DIR)/crt%.o: arch/i386/crt%.asm
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile ASM source file: $@'
	$(V) $(AS) $@ $< $(S_FLAGS)

endif


$(BOOT_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LDK  - Link kernel image: $@'
	$(V) $(LDK) $@ $^ $(LBK)

$(BIN_DIR)/%.xe:
	@ mkdir -p $(dir $@)
	$(E) '  LDX  - Link cross-program: $@'
	$(V) $(LDX) $@ $^ $(L_FLAGS) $(LIB)

$(BIN_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LD   - Link program: $@'
	$(V) $(LD) $@ $^ $(L_FLAGS) $(LIB)

$(LIB_DIR)/lib%.so:
	@ mkdir -p $(dir $@)
	$(E) '  LDS  - Link shared library: $@'
	$(V) $(LDS) $@ $^ $(L_FLAGS) $(LIB)

$(LIB_DIR)/lib%.a:
	@ mkdir -p $(dir $@)
	$(E) '  AR   - Link static archive: $@'
	$(V) $(AR) $@ $^




$(BUILD_DIR)/%.iso:
	@ mkdir -p $(@:.iso=/iso)
	$(V) echo $^ | tr ' ' '\n' | xargs -I % cp --parents -t $(@:.iso=/iso) % 
	$(V) grub-mkrescue -o $@ $(@:.iso=/iso) -A $(@:.iso=) > /dev/null
#	@ rm -rf $(@:.iso=/iso)

$(BOOT_DIR)/grub/grub.cfg: 
	@ mkdir -p $(dir $@)
	$(V) cp scripts/grub.cfg $@



# Typeof delivery
#  boot/image
#  bin/check.ut
#  bin/check.xe







# objs = $(patsubst src/%.c,$(OBJS_DIR)/%.o,$(1))
define objs
	$(patsubst src/%.c,$(OBJS_DIR)/%.o,    \
	$(patsubst src/%.cpp,$(OBJS_DIR)/%.o,  \
	$(patsubst src/%.asm,$(OBJS_DIR)/%.o,  \
	$(1)                                   \
	)))
endef


KRN_SRC = $(wildcard src/syscalls/*.c) \
					$(wildcard src/start/*.c) \
					$(wildcard src/vfs/*.c) \
					$(wildcard src/inodes/*.c) \
					$(wildcard src/stream/*.c) \
					$(wildcard src/memory/*.c) \
					$(wildcard src/task/*.c) \
					$(wildcard src/sys/*.c) \
					$(wildcard src/arch/i386/*.c) \
					$(wildcard src/scheduler/*.c) \
					$(wildcard src/fs/ata/*.c) \
					$(wildcard src/fs/vba/*.c) \
					$(wildcard src/fs/iso/*.c) \
					$(wildcard src/core/*.c)

CRTK = $(OBJS_DIR)/crtk.o
CRT0 = $(OBJS_DIR)/crt0.o

MST_SRC = $(wildcard src/dummy/master.c)

# ----------------------------------------------------


sources: 
	@ echo $(call objs,$(KRN_SRC))

clean:
	$(V) rm -rf $(OBJS_DIR)


destroy: clean
	$(V) rm -rf $(BIN_DIR)
	$(V) rm -rf $(LIB_DIR)
	$(V) rm -rf $(BOOT_DIR)


$(BIN_DIR)/master.xe: $(call objs,$(MST_SRC)) $(AXLIBC)/lib/libaxc.a



cdrom: $(BUILD_DIR)/OsCore.iso

$(BUILD_DIR)/OsCore.iso: $(BOOT_DIR)/kImage $(BOOT_DIR)/kImage.map $(BOOT_DIR)/grub/grub.cfg $(BIN_DIR)/master.xe




$(LIB_DIR)/libkern.so: $(call objs,$(KRN_SRC))

$(BOOT_DIR)/kImage: $(CRTK) $(call objs,$(KRN_SRC))




ifeq ($(MAKECMDGOALS),clean)
NODEPS = 1
endif
ifeq ($(MAKECMDGOALS),destroy)
NODEPS = 1
endif

ifeq ($(NODEPS),)
-include $(DEPS)
endif

