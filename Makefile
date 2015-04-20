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

# KRN_SRC = $(wildcard src/syscalls/*.c) \
# 					$(wildcard src/start/*.c) \
# 					$(wildcard src/vfs/*.c) \
# 					$(wildcard src/stream/*.c) \
# 					$(wildcard src/memory/*.c) \
# 					$(wildcard src/task/*.c) \
# 					$(wildcard src/sys/*.c) \
# 					$(wildcard src/scheduler/*.c) \
# 					$(wildcard src/fs/ata/*.c) \
# 					$(wildcard src/fs/vba/*.c) \
# 					$(wildcard src/fs/iso/*.c) 

MIN_SRC = $(wildcard src/start/*.c) \
					$(wildcard src/minimal/*.c) 

ARC_SRC =   $(wildcard src/arch/i386/*.c) \
            $(wildcard src/cpu/_x86/*.c)  \
					  $(wildcard src/runtime/*.c)

CHK_SRC =   $(wildcard src/check/*.c)

MST_SRC = $(wildcard src/dummy/master.c)

UM_SRC = $(wildcard src/*.c) \
 				 $(wildcard src/fs/*.c) \
 				 $(wildcard src/libc/*.c) \
 				 $(wildcard src/_um/*.c)

KRN_SRC = $(wildcard src/*.c) \
					$(wildcard src/fs/*.c) \
					$(wildcard src/libc/*.c) \
					$(wildcard src/_x86/*.c)

include scripts/global_commands.mk

# ----------------------------------------------------
cdrom: $(BUILD_DIR)/OsCore.iso

crtk: $(CRTK)
crt0: $(CRT0)


ifeq ($(MIN),)
$(BOOT_DIR)/kImage: $(CRTK) $(call objs,kernel,$(KR2_SRC)) 

# $(BOOT_DIR)/kImage: $(CRTK) \
# 		$(call objs,kernel,$(KR2_SRC)) 
#		$(AXLIBC)/lib/libAxRaw.a
# $(BOOT_DIR)/kImage: $(CRTK) \
# 		$(call objs,kernel,$(KRN_SRC)) \
# 		$(call objs,kernel,$(ARC_SRC)) \
# 		$(AXLIBC)/lib/libAxRaw.a
else
$(BOOT_DIR)/kImage: $(CRTK) \
		$(call objs,kernel,$(MIN_SRC)) \
		$(call objs,kernel,$(ARC_SRC)) \
		$(AXLIBC)/lib/libAxRaw.a
endif

$(BIN_DIR)/master.xe: $(call objs,smokeos,src/dummy/master.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/deamon.xe: $(call objs,smokeos,src/dummy/deamon.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/hello.xe: $(call objs,smokeos,src/dummy/hello.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/sname.xe: $(call objs,smokeos,src/dummy/sname.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/init.xe: $(call objs,smokeos,src/dummy/init.c) $(AXLIBC)/lib/libaxc.a
$(BIN_DIR)/kt_itimer.xe: $(call objs,smokeos,src/dummy/kt_itimer.c) $(AXLIBC)/lib/libaxc.a

$(BUILD_DIR)/OsCore.iso: $(BOOT_DIR)/grub/grub.cfg \
	$(BOOT_DIR)/kImage \
	$(BIN_DIR)/master.xe											\
	$(BIN_DIR)/deamon.xe											\
	$(BIN_DIR)/hello.xe												\
	$(BIN_DIR)/sname.xe												\
	$(BIN_DIR)/init.xe												\
	$(BIN_DIR)/kt_itimer.xe

# $(BOOT_DIR)/kImage.map \

kernSim:$(BIN_DIR)/kernSim

$(BIN_DIR)/kernSim: $(call objs,debug,$(KRN_SRC)) $(call objs,debug,$(CHK_SRC))


um:$(TEST_DIR)/um

$(TEST_DIR)/um: $(call objs,testing,$(UM_SRC))


# =======================================================
#      Code coverage HTML
# =======================================================

SED_LCOV  = -e '/SF:\/usr.*/,/end_of_record/d'
SED_LCOV += -e '/SF:.*\/src\/tests\/.*/,/end_of_record/d'

# -----------------------------	--------------------------
# Create coverage HTML report
%.lcov: $(TEST_DIR)/%
	@ find -name *.gcda | xargs -r rm
	@ CK_FORK=no $<
	@ lcov -c --directory . -b . -o $@
	@ sed $(SED_LCOV) -i $@

cov_%: %.lcov
	@ genhtml -o $@ $<


# =======================================================
#      Code quality reports
# =======================================================

# -------------------------------------------------------
# Static report - depend of sources
report_cppcheck_%.xml: src/%
	@ cppcheck -v --enable=all --xml-version=2 -Iinclude $(wildcard $</*) 2> $@

report_rats_%.xml: src/%
	@ rats -w 3 --xml $(wildcard $</*) > $@


# -------------------------------------------------------
# Dynamic report - depend of tests
report_check_%.xml: $(TEST_DIR)/%
	@ $< -xml

report_gcov_%.xml: $(TEST_DIR)/%
	@ find -name *.gcda | xargs -r rm
	@ CK_FORK=no $<
	@ gcovr -x -r . > $@

report_valgrind_%.xml: $(TEST_DIR)/%
	@ CK_FORK=no valgrind --xml=yes --xml-file=$@  $<


# -------------------------------------------------------
# -------------------------------------------------------
