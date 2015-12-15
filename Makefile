#      This file is part of the SmokeOS project.
#  Copyright (C) 2015  <Fabien Bavent>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#  This makefile is more or less generic.
#  The configuration is on `sources.mk`.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
NAME = SmokeOS_Kernel
ARCH = x86
LINUX = $(shell uname -sr)
DATE = $(shell date '+%d %b %Y')
GIT = $(shell git --git-dir=$(topdir)/.git log -n1 --pretty='%h')$(shell if [ -n "$(git --git-dir=$(topdir)/.git status --short -uno)"]; then echo '+'; fi)
VERSION = 0.1-$(GIT)

S = @
V = $(shell [ -z $(VERBOSE) ] && echo @)
Q = $(shell [ -z $(QUIET) ] && echo @ || echo @true)

all: libs utils

# D I R E C T O R I E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
prefix ?= /usr/local
topdir ?= $(shell readlink -f .)
gendir ?= $(shell pwd)
srcdir = $(topdir)/src
outdir = ${gendir}/obj
bindir = ${gendir}/bin
libdir = ${gendir}/lib

# C O M M A N D S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CROSS_COMPILE ?= $(CROSS)
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc -m32 
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld -melf_i386
NM = $(CROSS_COMPILE)nm

# A V O I D   D E P E N D E N C Y -=-=-=-=-=-=-=-=-=-=-=-
ifeq ($(shell [ -d $(outdir) ] || echo N ),N)
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

# D E L I V E R I E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
define obj
  $(patsubst $(srcdir)/%.c,$(outdir)/$(1)/%.$(3),   \
  $(patsubst $(srcdir)/%.cpp,$(outdir)/$(1)/%.$(3), \
  $(patsubst $(srcdir)/%.asm,$(outdir)/$(1)/%.$(3), \
    $(filter-out $($(2)_omit-y),$($(2)_src-y))      \
  )))
endef

define link
DEPS += $(call obj,$2,$1,d)
$1: $(bindir)/$1
$(bindir)/$1: $(call obj,$2,$1,o)
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    LD  "$$@
	$(V) $(CC) $($(1)_LFLAGS) -o $$@ $$^
	$(Q) ls -lh $$@
	$(Q) size $$@
endef

define kimg
DEPS += $(call obj,$2,$1,d)
$1: $(gendir)/$1
$(gendir)/$1: $(call obj,$2,$1,o) $(outdir)/_$(ARCH)/crtk.o
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    LD  "$$@
	$(V) $(LD) -T $(srcdir)/_$(ARCH)/kernel.ld $($(1)_LFLAGS) \
		-o $$@ -Map $$@.map $(call obj,$2,$1,o)
	$(Q) ls -lh $$@
	$(Q) size $$@
endef

define ccpl
$(outdir)/$(1)/%.o: $(srcdir)/%.c
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    CC  "$$@
	$(V) $(CC) -c $($(1)_CFLAGS) -o $$@ $$<
$(outdir)/$(1)/%.d: $(srcdir)/%.c
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    CM  "$$@
	$(V) $(CC) -c $($(1)_CFLAGS) -o $$@ $$<
endef

define crt
$(outdir)/%/$1.o: $(srcdir)/%/crt/$1.asm
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    ASM "$$@
	$(V) nasm -f elf32 -o $$@ $$^
endef

$(lib_dir)/lib%.a:
	$(S) mkdir -p $(dir $@)
	$(Q) echo "    AR  "$@
	$(V) ar src $@ $^

# S O U R C E S   C O M P I L A T I O N -=-=-=-=-=-=-=-=-
include $(topdir)/sources.mk

# C O M M O N   T A R G E T S -=-=-=-=-=-=-=-=-=-=-=-=-=-
libs: $(DV_LIBS)
utils: $(DV_UTILS) $(DV_CHECK)
statics: $(patsubst %,$(gendir)/lib/%.a,$(DV_LIBS))
# install: install_dev install_runtime install_utils
# unistall:
# TODO -- rm $(prefix)/XX (without messing with others libs)
clean: 
	$(V) rm -rf $(outdir)
distclean: clean
	$(V) rm -rf $(libdir)
	$(V) rm -rf $(bindir)
config:
# TODO -- Create/update configuration headers
check: $(DV_CHECK)
# TODO -- Launch unit tests
.PHONY: all libs utils install unistall 
.PHONY: clean distclean config check

# P A C K A G I N G -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
release: $(gendir)/$(NAME)-$(ARCH)-$(VERSION).tar
# .bz2
# $(gendir)/$(NAME)-$(ARCH)-$(VERSION).tar.bz2: $(gendir)/$(NAME)-$(ARCH)-$(VERSION).tar
# 	$(V) gzip $< -o $@
$(gendir)/$(NAME)-$(ARCH)-$(VERSION).tar: $(DV_UTILS) $(DV_LIBS)
	$(Q)  "  TAR   $@"
	$(V) tar cf $@  -C $(topdir) $(topdir)/include
	$(V) tar af $@ -C $(gendir) $^

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
deps:
	$(S) echo $(DEPS)

dirs:
	@ echo GPATH: $(GPATH)
	@ echo VPATH: $(VPATH)
	@ echo SPATH: $(SPATH)

delv:
	@ echo LIBS: $(DV_LIBS)
	@ echo UTILS: $(DV_UTILS)

ifeq ($(NODEPS),)
-include $(DEPS)
endif
