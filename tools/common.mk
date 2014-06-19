

all: deliveries


# Rules program ----------------------------------------------------------

$(bin_dir)/%$(bin_ext): 
	$(VVV) $(MKDIR) $(dir $@)
	$(E) 'Creation of the program $(notdir $@)'
	$(V) $(LD) -o $@ $(call tbinname,$@,_obj) $(call tbinname,$@,_lflags)

$(prefix)/bin/%: $(bin_dir)/%$(bin_ext)
	$(VVV) $(MKDIR) $(dir $@)
	$(E) Install program $@
	$(V) $(SU) cp $< $@
	$(VVV) $(SU) chmod 755 $@


# Rules library ----------------------------------------------------------

$(lib_dir)/%$(lib_ext): 
	$(VVV) $(MKDIR) $(dir $@)
	$(E) 'Creation of the library $(notdir $@)'
	$(V) $(LD) -shared -o $@ $($(patsubst %$(lib_ext),%, $(notdir $@))_obj) $($(patsubst %$(lib_ext),%, $(notdir $@))_lflags)
	$(VV) $(AR) $(@:%.so=%.a) $($(patsubst %$(lib_ext),%, $(notdir $@))_obj)  

$(prefix)/lib/%$(lib_ext): $(lib_dir)/%$(lib_ext)
	$(VVV) $(MKDIR) $(dir $@)
	$(EE) Install library $(notdir $@)
	$(VV) $(SU) cp $< $@.$(call tlibname,$@,_maj).$(call tlibname,$@,_min)
	$(VVV) $(SU) chmod 755 $@.$(call tlibname,$@,_maj).$(call tlibname,$@,_min)
	$(VVV) $(SU) ln -fs $(notdir $@).$(call tlibname,$@,_maj).$(call tlibname,$@,_min) $@.$(call tlibname,$@,_maj) || true
	$(VVV) $(SU) ln -fs $(notdir $@).$(call tlibname,$@,_maj) $@ || true

$(prefix)/lib/%.a: $(lib_dir)/%.a
	$(VVV) $(MKDIR) $(dir $@)
	$(EEE) Install library $(notdir $@)
	$(VVV) $(SU) cp $< $@

$(out_dir)/lib/%$(lib_ext): $(lib_dir)/%$(lib_ext)
	$(VVV) $(MKDIR) $(dir $@)
	$(EE) Create dummy library $(notdir $@)
	$(VV) objcopy -j .text -j .data -j .bss $< $@ 



# --------------- --------------- --------------- ---------------
 
$(obj_dir)/%.o: src/%.c $(obj_dir)/%.d
	$(VVV) $(MKDIR) $(dir $@)
	$(VV) $(CC) -c -o $@ $< $(CFLAGS)

$(obj_dir)/%.d: src/%.c
	$(VVV) $(MKDIR) $(dir $@)
	$(VVV) $(CC) -MM -MG -MT '$@ $(@:.d=.o)' -o $@ $< $(CFLAGS)




# ===========================================================================
# ===========================================================================

# Common Template -----------------------------------------------------------

define COMMON

$(1)_cflags += $(foreach dir, $($(1)_inc), -I$(dir))

$(1)_odir = $(obj_dir)/$(1)

$(1)_src += $(foreach dir, \
  $($(1)_frags:%=$(src_out)/%/$(src_dir)), \
  $(wildcard $(dir)/*$(src_ext)))

$(1)_obj0 = $$($(1)_src)
$(1)_obj1 = $$(patsubst %.c, $$($(1)_odir)/%.o, $$($(1)_obj0))
$(1)_obj2 = $$(patsubst %.cpp, $$($(1)_odir)/%.o, $$($(1)_obj1))
$(1)_obj = $$($(1)_obj2)

$(1)_dep = $$(patsubst %.o, %.d, $$($(1)_obj))

deps += $$($(1)_dep)


$$($(1)_odir)/%.o: $(src_top)/%.c  $$($(1)_odir)/%.d
	$(VVV) $(MKDIR) $$(dir $$@)
	$(EE) 'Compile c source $$<'
	$(VV) $(CC) -c -o $$@ $$< $$($(1)_cflags) 

$$($(1)_odir)/%.d: $(src_top)/%.c
	$(VVV) $(MKDIR) $$(dir $$@)
	$(EEE) 'Depandancy check for $$<'
	$(VVV) $(CC) -MM -MG -MT '$$@ $$(@:.d=.o)' -o $$@ $$< $$($(1)_cflags) 


$$($(1)_odir)/%.o: $(src_top)/%.cpp  $$($(1)_odir)/%.d
	$(VVV) $(MKDIR) $$(dir $$@)
	$(EE) 'Compile cpp source $$<'
	$(VV) $(CPP) -c -o $$@ $$< $$($(1)_cflags) 

$$($(1)_odir)/%.d: $(src_top)/%.cpp
	$(VVV) $(MKDIR) $$(dir $$@)
	$(EEE) 'Depandancy check for $$<'
	$(VVV) $(CPP) -MM -MG -MT '$$@ $$(@:.d=.o)' -o $$@ $$< $$($(1)_cflags) 

e_$(1):
	@ echo $$($(1)_src)
	@ echo $$($(1)_obj)
	@ echo $$($(1)_dep)

endef

# Deliveries Templates -------------------------------------------------------

define CRT 

$$($(1)_out): $$($(1)_src)
	$(VVV) $(MKDIR) $(dir $$@)
	$(EE) 'Compile cRuntime $(notdir $$<)'
	$(VV) nasm -f elf32 -o $$@ $$<
	
endef


define KERNEL
deliveries_bin += $(1)
$(eval $(call COMMON,$(1)))

$(bin_dir)/$(1)$(bin_ext): $($(1)_refs) $($(1)_obj) $($(1)_crt)
	$(VVV) $(MKDIR) $(bin_dir)
	$(E) 'Creation of the kernel image $(notdir $$@)'
	$(V) ld --oformat=binary -Map $(1).map -o $$@ -Ttext 20000 $($(1)_crt) $$(call tbinname,$$@,_obj) $$(call tbinname,$$@,_lflags) 
	
$(1): $(bin_dir)/$(1)$(bin_ext) 
endef

define KPROGRAM
deliveries_bin += $(1)
$(eval $(call COMMON,$(1)))

$(bin_dir)/$(1)$(bin_ext): $($(1)_refs) $($(1)_obj) $($(1)_crt)
	$(VVV) $(MKDIR) $(bin_dir)
	$(E) 'Creation of the program <ld-cross-platform> $(notdir $$@)'
	$(V) ld --oformat=elf32-i386 -o $$@ -Ttext 400000 $($(1)_crt) $$(call tbinname,$$@,_obj) $$(call tbinname,$$@,_lflags)
	
$(1): $(bin_dir)/$(1)$(bin_ext) 
endef


define PROGRAM
deliveries_bin += $(1)
$(eval $(call COMMON,$(1)))
$(bin_dir)/$(1)$(bin_ext): $($(1)_refs) $($(1)_obj)
$(1): $(bin_dir)/$(1)$(bin_ext) 
endef

define LIBRARY
deliveries_lib += $(1)
$(1)_maj ?= 1
$(1)_min ?= 0
$(eval $(call COMMON,$(1)))
$(lib_dir)/$(1)$(lib_ext): $($(1)_refs) $($(1)_obj)
$(1): $($(1)_refs) $(lib_dir)/$(1)$(lib_ext) 
  
endef

# --------------- --------------- --------------- ---------------



