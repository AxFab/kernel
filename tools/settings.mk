
prefix = /usr/
output = out/
arch ?= $(shell uname -m)
mode ?= debug
out_dir ?= Output_API

src_top = src

verbose = 0
explain = 0

# build directories ----------------------------------------------------------
bld_dir ?= .

bin_top ?= bin
bin_last ?= 
bin_dir = $(bld_dir)/$(bin_top)/$(arch)/$(mode)/$(bin_last)
bin_ext ?= 

lib_top ?= lib
lib_last ?= 
lib_dir = $(bld_dir)/$(lib_top)/$(arch)/$(mode)/$(lib_last)
lib_ext ?= .so

obj_top ?= obj
obj_last ?= 
obj_dir = $(bld_dir)/$(obj_top)/$(arch)/$(mode)/$(obj_last)


# defined function

tlibname = $($(patsubst %$(lib_ext),%,$(notdir $(1)))$(2))
tbinname = $($(patsubst %$(bin_ext),%,$(notdir $(1)))$(2))

# SU = su root 

# Verbose & Explain ----------------------------------------------------------
ifeq ($(shell [ $(verbose) -le '2' ] || echo 'Err'),)
VVV = @
endif

ifeq ($(shell [ $(verbose) -le 1 ] || echo 'Err'),)
VV = @
endif

ifeq ($(shell [ $(verbose) -le 0 ] || echo 'Err'),)
V = @
endif

ifeq ($(shell [ $(explain) -le '2' ] || echo 'Err'),)
EEE = @true
else
EEE = @echo
endif

ifeq ($(shell [ $(explain) -le 1 ] || echo 'Err'),)
EE = @true
else
EE = @echo
endif

ifeq ($(shell [ $(explain) -le 0 ] || echo 'Err'),)
E = @true
else
E = @echo
endif


MKDIR = mkdir -p 

ifeq ($(mode),debug)
LD = gcc
CC = gcc
CPP = g++
AR = ar rc
else 
LD = gcc
CC = gcc
CPP = g++
AR = ar rc
endif
