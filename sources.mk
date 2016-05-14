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
is_pc= $(shell [ $(target_arch) = x86 ] && echo y)

# M O D U L E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# Kernel sources
mod_krn-y += $(wildcard $(srcdir)/util/*.c)
mod_krn-y += $(wildcard $(srcdir)/scall/*.c)
mod_krn-y += $(wildcard $(srcdir)/mem/*.c)
mod_krn-y += $(wildcard $(srcdir)/stm/*.c)
mod_krn-y += $(wildcard $(srcdir)/tsk/*.c)
mod_krn-y += $(wildcard $(srcdir)/vfs/*.c)

# Minimum file systems
mod_mfs-y += $(wildcard $(srcdir)/vfs/gpt/*.c)
mod_mfs-y += $(wildcard $(srcdir)/vfs/iso/*.c)
mod_mfs-y += $(wildcard $(srcdir)/vfs/kdb/*.c)
mod_mfs-y += $(wildcard $(srcdir)/vfs/fat/*.c)

# Configured file systems
mod_kfs-y += $(mod_mfs-y)
mod_kfs-$(is_pc) += $(wildcard $(srcdir)/vfs/ata/*.c)
mod_kfs-$(is_pc) += $(wildcard $(srcdir)/vfs/svga/*.c)


# F L A G S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CFLAGS += -Wall -Wextra -Wno-unused-parameter 
CFLAGS += -D_DATE_=\"'$(DATE)'\" -D_OSNAME_=\"'$(LINUX)'\"
CFLAGS += -D_GITH_=\"'$(GIT)'\" -D_VTAG_=\"'$(VERSION)'\"

ut_LFLAGS += --coverage
ut_LIBS += `pkg-config --libs check`
ut_CFLAGS += -ggdb3 --coverage $(CFLAGS) -D_FS -D_FS_UM
ut_CFLAGS += -I $(topdir)/include
ut_CFLAGS += -I $(topdir)/include/asm/_um

kum_LFLAGS += --coverage
kum_CFLAGS += -ggdb3 --coverage $(CFLAGS) -D_FS -D_FS_UM
kum_CFLAGS += -I $(topdir)/include
kum_CFLAGS += -I $(topdir)/include/asm/_um

krn_$(target_arch)_LFLAGS += -nostdlib 
krn_$(target_arch)_CFLAGS += $(CFLAGS) -ggdb3
krn_$(target_arch)_CFLAGS += -I $(topdir)/include
krn_$(target_arch)_CFLAGS += -I $(topdir)/include/asm/_$(target_arch)


# D E L I V E R I E S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# Kernel sources
kImage_src-y += $(mod_krn-y) $(mod_kfs-y)
kImage_src-y += $(wildcard $(srcdir)/libc/*.c)
kImage_src-y += $(wildcard $(srcdir)/asm/_$(target_arch)/*.c)


# kernel usermode (functional tests)
kum_src-y += $(mod_krn-y) $(mod_mfs-y)
kum_src-y += $(wildcard $(srcdir)/asm/_um/*.c)
kum_src-y += $(wildcard $(srcdir)/vfs/bmp/*.c)
kum_src-y += $(wildcard $(srcdir)/vfs/hdd/*.c)


# U N I T - T E S T S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# UT_ (unit tests)
ut_src-y += $(wildcard $(srcdir)/test/*.c)
ut_src-y += $(mod_krn-y) $(mod_mfs-y)
ut_src-y += $(wildcard $(srcdir)/asm/_um/*.c)
ut_src-y += $(wildcard $(srcdir)/vfs/bmp/*.c)
ut_src-y += $(wildcard $(srcdir)/vfs/hdd/*.c)
ut_omit-y += $(srcdir)/asm/_um/start.c

# T A R G E T S -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
$(eval $(call ccpl,ut)) # compile unit test
$(eval $(call link,ut,ut)) # link unit test

$(eval $(call ccpl,kum)) # compile hosted kernel
$(eval $(call link,kum,kum)) # link hosted kernel

$(eval $(call ccpl,krn_$(target_arch))) # compile kernel image
$(eval $(call kimg,kImage,krn_$(target_arch))) # link kernel image

$(eval $(call crt,crtk)) # compile CRTK

DV_CHECK += $(bindir)/ut
DV_UTILS += $(bindir)/kum  
# $(gendir)/kImage
