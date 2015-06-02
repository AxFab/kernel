# libkfs.a - internal
# ---------------------------------------------------------------------------
ifeq ($(obj_dir),)
$(error  This file is part of a larger script!)
else

SRC_kfs += $(src_dir)/src/fs/gpt.c
SRC_kfs += $(src_dir)/src/fs/iso.c
SRC_kfs += $(src_dir)/src/fs/kdb.c
# SRC_kfs += $(src_dir)/src/fs/raid0.c
SRC_kfs += $(src_dir)/src/fs/fat.c
SRC_kfs += $(src_dir)/src/fs/tmpfs.c

ifeq ($(ARCH),um)
SRC_kfs += $(src_dir)/src/fs/bmp.c
SRC_kfs += $(src_dir)/src/fs/hdd.c
endif

ifeq ($(ARCH),x86)
SRC_kfs += $(src_dir)/src/fs/ata.c
SRC_kfs += $(src_dir)/src/fs/svga.c
endif

$(lib_dir)/libkfs.a: $(call objs,$(SRC_kfs))

endif
