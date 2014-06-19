# Makefile 

#ifeq ($(MAKE_SCRIPTS_DIR),)
  MAKE_SCRIPTS_DIR = tools
#endif

-include $(MAKE_SCRIPTS_DIR)/settings.mk
-include $(MAKE_SCRIPTS_DIR)/common.mk


# ===========================================================================

linuxname = $(shell uname -sr)
date = $(shell date '+%d %b %Y')

pack_cflags =  -D_DATE_=\"'$(date)'\" -D_OS_FULLNAME_=\"'$(linuxname)'\"

cflags = $(pack_cflags) -Wall -Wextra -Wno-unused-parameter

std_debug_cflags = $(cflags) -g -ggdb
std_cov_cflags = $(cflags) --coverage -fprofile-arcs -ftest-coverage
std_release_cflags = -fPIC -O3 $(cflags)


std_cov_lflags = $(lflags) -fprofile-arcs


# ===========================================================================


# Target: Program  inodes
inodes_src = $(patsubst src/%,%,$(wildcard src/inodes/*.c)) \
			       $(patsubst src/%,%,$(wildcard src/core/*.c)) \
			       $(patsubst src/%,%,$(wildcard src/fs/tmpfs/*.c)) \
			       $(patsubst src/%,%,$(wildcard src/fs/img/*.c)) \
			       $(patsubst src/%,%,$(wildcard src/fs/iso9660/*.c)) \
			       dbg/inodes.c
inodes_inc = include/ 
inodes_cflags = $(std_$(mode)_cflags)
inodes_lflags = $(std_$(mode)_lflags)
$(eval $(call PROGRAM,inodes))

# Target: Program  memory
memory_src = $(patsubst src/%,%,$(wildcard src/memory/*.c)) \
			       $(patsubst src/%,%,$(wildcard src/core/*.c)) \
			       dbg/memory.c
memory_inc = include/ 
memory_cflags = $(std_$(mode)_cflags)
memory_lflags = $(std_$(mode)_lflags)
$(eval $(call PROGRAM,memory))



# ===========================================================================

-include $(MAKE_SCRIPTS_DIR)/utils.mk

