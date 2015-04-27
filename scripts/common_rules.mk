# =======================================================
# Generic makefile 'common_rules.mk'
#     Fabien B. <fabien.bavent@gmail.com>
#
# This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# =======================================================


# *** Intermediate debug files ***

$(OBJS_DIR)/debug/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS_DEBUG) $(INC_DEBUG) $(DEF_DEBUG)

$(OBJS_DIR)/debug/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $< $(C_FLAGS_DEBUG) $(INC_DEBUG) $(DEF_DEBUG) > $@
	@ cp $@ $@.tmp
	@ cat $@.tmp | fmt -1 | sed -e 's/.*://' -e 's/\$//' -e 's/^\s*//' | grep -v '^\s*$' | sed 's/$/:/' >> @$
	@ rm $@.tmp

$(OBJS_DIR)/debug/%.o: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXX  - Compile C++ source file: $@'
	$(V) $(CXX) $@ $< $(CXX_FLAGS_DEBUG) $(INC_DEBUG) $(DEF_DEBUG)

$(OBJS_DIR)/debug/%.d: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXXD - Dependancies of C++ source file: $@'
	$(V) $(CXXD) $< $(CXX_FLAGS_DEBUG) $(INC_DEBUG) $(DEF_DEBUG) > $@
	@ cp $@ $@.tmp
	@ cat $@.tmp | fmt -1 | sed -e 's/.*://' -e 's/\$//' -e 's/^\s*//' | grep -v '^\s*$' | sed 's/$/:/' >> @$
	@ rm $@.tmp



# *** Intermediate release files ***

$(OBJS_DIR)/release/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS_RELEASE) $(INC_RELEASE) $(DEF_RELEASE)

$(OBJS_DIR)/release/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $@ $< $(C_FLAGS_RELEASE) $(INC_RELEASE) $(DEF_RELEASE)

$(OBJS_DIR)/release/%.o: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXX  - Compile C++ source file: $@'
	$(V) $(CXX) $@ $< $(CXX_FLAGS_RELEASE) $(INC_RELEASE) $(DEF_RELEASE)

$(OBJS_DIR)/release/%.d: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXXD - Dependancies of C++ source file: $@'
	$(V) $(CXXD) $@ $< $(CXX_FLAGS_RELEASE) $(INC_RELEASE) $(DEF_RELEASE)


# *** Intermediate testing files ***

$(OBJS_DIR)/testing/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Compile C source file: $@'
	$(V) $(CC) $@ $< $(C_FLAGS_TESTING) $(INC_TESTING) $(DEF_TESTING)

$(OBJS_DIR)/testing/%.d: $(SOURCE_DIR)/%.c
	@ mkdir -p $(dir $@)
	$(E) '  CC   - Dependancies of C source file: $@'
	$(V) $(CCD) $@ $< $(C_FLAGS_TESTING) $(INC_TESTING) $(DEF_TESTING)

$(OBJS_DIR)/testing/%.o: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXX  - Compile C++ source file: $@'
	$(V) $(CXX) $@ $< $(CXX_FLAGS_TESTING) $(INC_TESTING) $(DEF_TESTING)

$(OBJS_DIR)/testing/%.d: $(SOURCE_DIR)/%.cpp
	@ mkdir -p $(dir $@)
	$(E) '  CXXD - Dependancies of C++ source file: $@'
	$(V) $(CXXD) $@ $< $(CXX_FLAGS_TESTING) $(INC_TESTING) $(DEF_TESTING)


# *** Deliveries files ***

$(BIN_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LD   - Link program: $@'
	$(V) $(LD) $@ $^ $(L_FLAGS_DEBUG) $(LIB_DEBUG)

$(BINR_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LD   - Link program: $@'
	$(V) $(LD) $@ $^ $(L_FLAGS_RELEASE) $(LIB_RELEASE)

$(LIB_DIR)/lib%.so:
	@ mkdir -p $(dir $@)
	$(E) '  LDS  - Link shared library: $@'
	$(V) $(LDS) $@ $^ $(L_FLAGS_DEBUG) $(LIB_DEBUG)

$(LIBR_DIR)/lib%.so:
	@ mkdir -p $(dir $@)
	$(E) '  LDS  - Link shared library: $@'
	$(V) $(LDS) $@ $^ $(L_FLAGS_RELEASE) $(LIB_RELEASE)

$(LIB_DIR)/lib%.a:
	@ mkdir -p $(dir $@)
	$(E) '  AR   - Link static archive: $@'
	$(V) $(AR) $@ $^

$(TEST_DIR)/%:
	@ mkdir -p $(dir $@)
	$(E) '  LD   - Link test program: $@'
	$(V) $(LD) $@ $^ $(L_FLAGS_TESTING) $(LIB_TESTING)

