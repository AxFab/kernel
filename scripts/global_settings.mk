
ifeq ($(EXPLAIN),)
E = @ true
else
E = @ echo
endif

ifeq ($(VERBOSE),)
V = @
else
V = 
endif

ARCH ?= i386

CC = gcc -c -o
CCD = gcc -MM -o
AS = nasm -f elf32 -o

LD = gcc -o
AR = ar rc
LDS = gcc -shared -o
LDK = ld --oformat=binary -Map $@.map -Ttext 20000 -o
LDX = ld -T scripts/smoke.ld -o


# *** External info ***
linuxname := $(shell uname -sr)
git_hash := $(shell git log -n1 --pretty='%h')$(shell if [ -n "$(git status --short -uno)"]; then echo '+'; fi)
date := $(shell date '+%d %b %Y')
vtag := $(shell cat .build)



# *** Folders ***
BUILD_DIR = .
ORIGIN_DIR = .
OBJS_DIR = $(BUILD_DIR)/obj
SOURCE_DIR = $(ORIGIN_DIR)/src
BIN_DIR = $(BUILD_DIR)/bin
BOOT_DIR = $(BUILD_DIR)/boot
LIB_DIR = $(BUILD_DIR)/lib
BINR_DIR = $(BUILD_DIR)/release/bin
LIBR_DIR = $(BUILD_DIR)/release/lib
INCR_DIR = $(BUILD_DIR)/release/include

ALL_SRC  = $(wildcard src/*/*.c)
ALL_SRC += $(wildcard src/*/*/*.cpp)
ALL_SRC += $(wildcard src/*/*/*.c)
ALL_SRC += $(wildcard src/*/*.cpp)
ALL_SRC += $(wildcard src/*/$(ARCH)/*.asm)

ALL_INC  = $(wildcard include/*.h)
ALL_INC += $(wildcard include/*.hpp)
ALL_INC += $(wildcard include/*/*.h)
ALL_INC += $(wildcard include/*/*.hpp)

# *** 3rdParty Libraries ***
AXLIBC = $(ORIGIN_DIR)/3rdparty/axlibc

INC  = -I $(ORIGIN_DIR)/include -I arch/$(ARCH)/include/
INC += -I $(AXLIBC)/include/ -I $(AXLIBC)/internal/

DEF  = -D_DATE_=\"'$(date)'\" -D_OSNAME_=\"'$(linuxname)'\" 
DEF += -D_GITH_=\"'$(git_hash)'\" -D_VTAG_=\"'$(vtag)'\"


ASTYLE = --style=kr
ASTYLE += --indent=spaces=2
ASTYLE += --break-blocks
ASTYLE += --pad-oper
ASTYLE += --pad-header
ASTYLE += --align-pointer=name
ASTYLE += --convert-tabs
ASTYLE += --indent-col1-comments
# ASTYLE += --delete-empty-lines
# ASTYLE += --remove-brackets
# ASTYLE += --max-code-length=80 --break-after-logical



# *** Flags ***
CFLAGS = -Wall -Wextra -Wno-unused-parameter
LFLAGS = 


C_FLAGS_DEBUG = $(CFLAGS) -ggdb3
CXX_FLAGS_DEBUG = $(CFLAGS) -ggdb3
INC_DEBUG = $(INC)
DEF_DEBUG = $(DEF) -D__EX -D__x86_64__
L_FLAGS_DEBUG = $(LFLAGS)
LIB_DEBUG = 

C_FLAGS_RELEASE = $(CFLAGS) -O3
CXX_FLAGS_RELEASE = $(CFLAGS) -O3
INC_RELEASE = 
DEF_RELEASE = $(DEF)
L_FLAGS_RELEASE = $(LFLAGS)
LIB_RELEASE = 

C_FLAGS_TESTING = $(CFLAGS) --coverage -fprofile-arcs -ftest-coverage
CXX_FLAGS_TESTING = $(CFLAGS) --coverage -fprofile-arcs -ftest-coverage
INC_TESTING = 
DEF_TESTING = $(DEF)
L_FLAGS_TESTING = $(LFLAGS)
LIB_TESTING = -lcheck


S_FLAGS_CRT = 
C_FLAGS_KERNEL = $(CFLAGS) -nostdinc
CXX_FLAGS_KERNEL = $(CFLAGS) -nostdinc
INC_KERNEL = $(INC)
DEF_KERNEL = $(DEF) -D__EX -D__KERNEL -D__x86_64__
S_FLAGS_KERNEL = 

C_FLAGS_SMOKEOS = $(CFLAGS) -nostdinc
CXX_FLAGS_SMOKEOS = $(CFLAGS) -nostdinc
INC_SMOKEOS = $(INC)
DEF_SMOKEOS = $(DEF) -D__EX -D__KERNEL -D__x86_64__

L_FLAGS_KERNEL = $(LFLAGS)
L_FLAGS_SMOKEOS = $(LFLAGS)
LIB_SMOKEOS = 




