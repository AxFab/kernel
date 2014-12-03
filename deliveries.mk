
# =======================================================
# Makefile for Smoke-kernel
#     Fabien B. <fabien.bavent@gmail.com>
#
# This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# =======================================================

# -------------------------------------------------------
# Config
PACKAGE = kernel
VERSION = v0.1


define UT_PROGRAM
OBJS_$(1)UT = $(patsubst src/%.cpp,obj/testing/%.o, $(patsubst src/%.c,obj/testing/%.o, $(2)))
DEPS_$(1)UT = $(patsubst src/%.cpp,obj/testing/%.d, $(patsubst src/%.c,obj/testing/%.d, $(2)))
test/$(1)UT: $$(OBJS_$(1)UT)
TEST += $(1)UT
DEPS += $$(DEPS_$(1)UT)
endef

# =======================================================
#      Define deliveries
# =======================================================

CFLAGS_testing += -Iinclude -D__EX -Wno-unused-parameter -Iarch/i386/include
LFLAGS_testing +=

# -------------------------------------------------------
$(eval $(call UT_PROGRAM,vfs,    \
			$(wildcard src/vfs/*.c)    \
			$(wildcard src/fs/img/*.c) \
			$(wildcard src/fs/iso/*.c) \
			src/test/runtime.c         \
			src/test/vfs.c             \
		))

# # -------------------------------------------------------
$(eval $(call UT_PROGRAM,memory, \
			$(wildcard src/memory/*.c) \
			src/test/kpsize.c          \
			src/test/inode.c           \
			src/test/runtime.c         \
			src/test/mmu.c             \
			src/test/memory.c          \
		))

# # -------------------------------------------------------
$(eval $(call UT_PROGRAM,task,   \
			$(wildcard src/task/*.c)	 \
			src/test/runtime.c         \
			src/test/inode.c           \
			src/test/task.c            \
		))


# # -------------------------------------------------------
$(eval $(call UT_PROGRAM,aatree, \
			src/test/aatree.c          \
		))
