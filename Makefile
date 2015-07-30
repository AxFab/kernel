# Makefile
# ---------------------------------------------------------------------------

# Settings user
ARCH ?= x86
bld_dir ?= .
prefix =

CC = gcc
LD =  ld
V =

CFLAGS = -Wall -Wextra -Wno-unused-parameter -ggdb3
LFLAGS =
INC =
SRCS =

# Settings scripted
src_dir = $(shell dirname $(firstword $(MAKEFILE_LIST)))

arch_src = $(src_dir)/src/_$(ARCH)
deps_ax = $(src_dir)/3rdparty/axlibc
bin_dir = $(bld_dir)
lib_dir = $(bld_dir)/lib
obj_dir = $(bld_dir)/obj
krn_img = $(bld_dir)/$(prefix)/kImage


ifneq ($(ARCH),um)
CFLAGS += -ggdb3
# CFLAGS += -nostdinc -isystem $(deps_ax)/include
LFLAGS += -nostdlib
else
CFLAGS += -ggdb3 --coverage -fprofile-arcs -ftest-coverage
CFLAGS += -D_FS -D_FS_UM
LFLAGS += --coverage
endif


# ---------------------------------------------------------------------------
# Additional info
linuxname := $(shell uname -sr)
git_hash := $(shell git --git-dir=$(src_dir)/.git log -n1 --pretty='%h')$(shell if [ -n "$(git status --short -uno)"]; then echo '+'; fi)
date := $(shell date '+%d %b %Y')
vtag := $(shell cat $(src_dir)/.build)

CFLAGS += -D_DATE_=\"'$(date)'\" -D_OSNAME_=\"'$(linuxname)'\"
CFLAGS += -D_GITH_=\"'$(git_hash)'\" -D_VTAG_=\"'$(vtag)'\"

INC += -I $(src_dir)/include -I $(src_dir)/include/_$(ARCH)
INC += -I $(src_dir)/internal -I $(src_dir)/internal/_$(ARCH)

# ---------------------------------------------------------------------------
# Generic definitions
define objs
	$(patsubst $(src_dir)/src/%.c,$(obj_dir)/%.o,    \
	$(patsubst $(src_dir)/src/%.cpp,$(obj_dir)/%.o,  \
	$(patsubst $(src_dir)/src/%.asm,$(obj_dir)/%.o,  \
	$(1)                                             \
	)))
endef

ifeq ($(VERBOSE),)
V = @
else
V =
endif

ifeq ($(QUIET),)
Q = @ echo
else
Q = @ true
endif

ifeq ($(shell [ -d $(obj_dir) ] || echo N ),N)
NODEPS=1
endif
ifeq ($(MAKECMDGOALS),help)
NODEPS = 1
endif
ifeq ($(MAKECMDGOALS),clean)
NODEPS = 1
endif
ifeq ($(MAKECMDGOALS),distclean)
NODEPS = 1
endif

# ---------------------------------------------------------------------------
# Source code
SRCS += $(wildcard $(src_dir)/src/kern/*.c)
SRCS += $(wildcard $(src_dir)/src/_$(ARCH)/*.c)
SLIB += $(lib_dir)/libkfs.a

ifneq ($(ARCH),um)
SRCS += $(wildcard $(src_dir)/src/libc/*.c)
# SLIB += $(lib_dir)/libaxb.a
endif


# ---------------------------------------------------------------------------
all: $(krn_img)
#	@ strip $<
	@ ls -lh $<
	@ size $<

include $(arch_src)/make.mk
include $(src_dir)/src/fs/make.mk

$(krn_img): $(call objs,$(SRCS)) $(SLIB) $(KDEPS)
	@ mkdir -p $(dir $@)
	$(Q) "    LD  "$@
ifneq ($(ARCH),um)
	$(V) $(LD) -T $(arch_src)/kernel.ld $(LFLAGS) -o $@ -Map $@.map $(call objs,$(SRCS)) $(SLIB)
else
	$(V) $(CC) $(LFLAGS) -o $@ $^
endif

$(obj_dir)/%.o: $(src_dir)/src/%.c
	@ mkdir -p $(dir $@)
	$(Q) "    CC  "$<
	$(V) $(CC) -c $(INC) $(CFLAGS) -o $@ $<

$(obj_dir)/%.d: $(src_dir)/src/%.c
	@ mkdir -p $(dir $@)
	$(Q) "    DP  "$<
	$(V) $(CC) -MM $(INC) $(CFLAGS) -o $@ $<
ifneq ($(DPMAX),)
	@ cp $@ $@.tmp
	@ sh -c "cat $@.tmp | fmt -1 | sed -e 's/.*://' -e 's/\\\\$$//' -e 's/^\s*//' | grep -v '^\s*$$' | sed 's/$$/:/'" >> $@
	@ rm $@.tmp
endif

# ===========================================================================
#    Static Libraries
# ===========================================================================

# ---------------------------------------------------------------------------
# libaxb.a - external
$(lib_dir)/libaxb.a:
	$(Q) "    >>  "$@
	@ cd $(deps_ax) && $(MAKE) $@
	$(Q) "    <<  "$@


# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
$(lib_dir)/lib%.a:
	@ mkdir -p $(dir $@)
	$(Q) "    AR  "$@
	$(V) ar src $@ $^

clean:
	@ rm -rf $(obj_dir)
	@ rm -rf $(lib_dir)

distclean: clean
	@ rm -f $(krn_img)
	@ rm -rf cov_*

sources:
	@ echo $(SRCS) | tr ' ' '\n'

help:
	@ echo " "
	@ echo "        Makefile for the kernel"
	@ echo "USAGE:"
	@ echo "  make [target] [options]"
	@ echo " "
	@ echo " The target can be the path of a special delivery or one of "
	@ echo " the commands: "
	@ echo "   all:        build the kernel image"
	@ echo "   clean:      remove temporary file (objs & libs)"
	@ echo "   distclean:  remove all generated files."
	@ echo "   sources:    print a list of kernel sources files."
	@ echo "   help:       print this help."
	@ echo " Some other targets might be defined for one architecture:"
	@ echo "   _um: coverage (code coverage - you need testing repository)."
	@ echo "   _x86: cdrom, create a bootable ISO image."
	@ echo " "
	@ echo "OPTIONS:"
	@ echo "    QUIT=1      Remove extra logs."
	@ echo "    VERBOSE=1   Print main commands."
	@ echo "    NODEPS=1    Don't compute source dependencies."
	@ echo "                Note this option may activates by-itself."
	@ echo "    ARCH=?      Change the architecture. I recommand changing "
	@ echo "                also the build directory or cleaning (default:x86)."
	@ echo "    bld_dir=?   Change the directory to put generated file (default:.)."
	@ echo " "
	@ echo "DEPENDENCIES:"
	@ echo "  This script may run only using binutils suite."
	@ echo "  However architecture specific part will often require a bit more."
	@ echo "    - um  :: lcov"
	@ echo "    - x86 :: nasm (grub xorriso)"
	@ echo " "



# ===========================================================================

ifeq ($(NODEPS),)
-include $(patsubst %.o,%.d,$(call objs,$(SRCS)))
endif

# ===========================================================================
# ===========================================================================
