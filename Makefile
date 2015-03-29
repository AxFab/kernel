# Makefile for SmokeOS kernel
# ----------------------------------------------------

include scripts/global_settings.mk

all: cdrom $(BOOT_DIR)/kImage


clean:
	$(V) rm -rf $(OBJS_DIR)
	$(V) rm -rf $(patsubst %/iso,%,$(wildcard */iso))


destroy: clean
	$(V) rm -rf $(BIN_DIR)
	$(V) rm -rf $(LIB_DIR)
	$(V) rm -rf $(BOOT_DIR)
	$(V) rm -rf *.iso

include scripts/kernel_rules.mk
include scripts/common_rules.mk


# ----------------------------------------------------
CRTK = $(OBJS_DIR)/crtk.o
CRT0 = $(OBJS_DIR)/crt0.o

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

MST_SRC = $(wildcard src/dummy/master.c)

include scripts/global_commands.mk

# ----------------------------------------------------
cdrom: $(BUILD_DIR)/OsCore.iso

crtk: $(CRTK)
crt0: $(CRT0)

$(BOOT_DIR)/kImage: $(CRTK) $(call objs,kernel,$(KRN_SRC)) $(AXLIBC)/lib/libAxRaw.a

$(BIN_DIR)/master.xe: $(call objs,smokeos,src/dummy/master.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/deamon.xe: $(call objs,smokeos,src/dummy/deamon.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/hello.xe: $(call objs,smokeos,src/dummy/hello.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/sname.xe: $(call objs,smokeos,src/dummy/sname.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/kt_itimer.xe: $(call objs,smokeos,src/dummy/kt_itimer.c) $(AXLIBC)/lib/libaxc.a

$(BUILD_DIR)/OsCore.iso: $(BOOT_DIR)/grub/grub.cfg \
	$(BOOT_DIR)/kImage $(BOOT_DIR)/kImage.map \
	$(BIN_DIR)/master.xe											\
	$(BIN_DIR)/deamon.xe											\
	$(BIN_DIR)/hello.xe											\
	$(BIN_DIR)/sname.xe											\
	$(BIN_DIR)/kt_itimer.xe

