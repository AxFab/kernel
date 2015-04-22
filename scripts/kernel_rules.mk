# =======================================================
# Generic makefile 'kernel_rules.mk'
#     Fabien B. <fabien.bavent@gmail.com>
#
# This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# =======================================================


# *** C runtime ***

$(OBJS_DIR)/crt%.o: $(ORIGIN_DIR)/src/_x86/crt/crt%.asm
	@ mkdir -p $(dir $@)
	$(E) '  AS   - Compile C runtime: $@'
	$(V) $(AS) $@ $< $(S_FLAGS_CRT)


# *** Intermediate kernel sources ***

$(OBJS_DIR)/kernel/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS_KERNEL) $(INC_KERNEL) $(DEF_KERNEL)

$(OBJS_DIR)/kernel/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $@ $< $(C_FLAGS_KERNEL) $(INC_KERNEL) $(DEF_KERNEL)

$(OBJS_DIR)/kernel/%.o: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXX  - Compile C++ source file: $@'
	$(V) $(CXX) $@ $< $(CXX_FLAGS_KERNEL) $(INC_KERNEL) $(DEF_KERNEL)

$(OBJS_DIR)/kernel/%.d: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXXD - Dependancies of C++ source file: $@'
	$(V) $(CXXD) $@ $< $(CXX_FLAGS_KERNEL) $(INC_KERNEL) $(DEF_KERNEL)

$(OBJS_DIR)/kernel/%.o: $(SOURCE_DIR)/%.asm
	@ mkdir -p $(dir $@)
	$(E) '  AS   - Compile ASM source file: $@'
	$(V) $(AS) $@ $< $(S_FLAGS_KERNEL)


# *** Intermediate smokeos program files ***

$(OBJS_DIR)/smokeos/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS_SMOKEOS) $(INC_SMOKEOS) $(DEF_SMOKEOS)

$(OBJS_DIR)/smokeos/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $@ $< $(C_FLAGS_SMOKEOS) $(INC_SMOKEOS) $(DEF_SMOKEOS)

$(OBJS_DIR)/smokeos/%.o: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXX  - Compile C++ source file: $@'
	$(V) $(CXX) $@ $< $(CXX_FLAGS_SMOKEOS) $(INC_SMOKEOS) $(DEF_SMOKEOS)

$(OBJS_DIR)/smokeos/%.d: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXXD - Dependancies of C++ source file: $@'
	$(V) $(CXXD) $@ $< $(CXX_FLAGS_SMOKEOS) $(INC_SMOKEOS) $(DEF_SMOKEOS)


# *** Deliveries files ***

$(BOOT_DIR)/%.map: $(BOOT_DIR)/%
	$(V) objcopy --only-keep-debug $< $@

$(BOOT_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LD   - Link kernel image: $@'
	$(V) $(LDK) $@ $^ $(L_FLAGS_KERNEL)

$(BIN_DIR)/%.xe:
	@ mkdir -p $(dir $@)
	$(E) '  LDX  - Link cross-program: $@'
	$(V) $(LDX) $@ $^ $(L_FLAGS_SMOKEOS) $(LIB_SMOKEOS)

$(LIB_DIR)/lib%.xo:
	@ mkdir -p $(dir $@)
	$(E) '  LDS  - Link cross-shared library: $@'
	$(V) $(LDS) $@ $^ $(L_FLAGS_SMOKEOS) $(LIB_SMOKEOS)


# *** Disk image rules ***

$(BUILD_DIR)/%.iso:
	@ mkdir -p $(@:.iso=/iso)
	$(V) echo $^ | tr ' ' '\n' | xargs -I % cp --parents -t $(@:.iso=/iso) % 
	$(V) grub-mkrescue -o $@ $(@:.iso=/iso) > /dev/null
#	$(V) grub-mkrescue -o $@ $(@:.iso=/iso) -A $(@:.iso=) > /dev/null
# @ rm -rf $(@:.iso=/iso)

$(BOOT_DIR)/grub/grub.cfg: 
	@ mkdir -p $(dir $@)
	$(V) cp scripts/grub.cfg $@

