

deliveries_name: $(deliveries_lib) $(deliveries_bin)

deliveries: $(foreach lib,$(deliveries_lib), $(lib_dir)/$(lib)$(lib_ext)) \
	$(foreach bin,$(deliveries_bin), $(bin_dir)/$(bin)$(bin_ext))

install: $(foreach lib,$(deliveries_lib), $(prefix)/lib/$(lib)$(lib_ext)) \
	$(foreach lib,$(deliveries_lib), $(prefix)/lib/$(lib).a) \
	$(foreach bin,$(deliveries_bin), $(prefix)/bin/$(bin)$(bin_ext))

output: $(foreach lib,$(deliveries_lib), $(output)/lib/$(lib)$(lib_ext))

config:
	@ echo 'Executable build output: '$(bin_dir)
	@ echo 'Library build output: '$(lib_dir)
	@ echo 'Temporary files output: '$(obj_dir)
	@ echo 'All names: '$(deliveries_lib) $(deliveries_bin)
	@ echo 'All: '$(foreach lib,$(deliveries_lib), $(lib_dir)/$(lib)$(lib_ext)) \
		$(foreach bin,$(deliveries_bin), $(bin_dir)/$(bin)$(bin_ext))
	@echo 'Install: '$(foreach lib,$(deliveries_lib), $(lib_dir)/$(lib)$(lib_ext)) \
	$(foreach lib,$(deliveries_lib), $(lib_dir)/$(lib).a) \
	$(foreach bin,$(deliveries_bin), $(bin_dir)/$(bin)$(bin_ext))
	@echo 'Output: '$(foreach lib,$(deliveries_lib), $(output)/lib/$(lib)$(lib_ext))
	@echo 'Dependancies files: '$(deps)


clean:
	$(VVV) rm -rf $(obj_top)

mrproper: clean
	$(VVV) rm -rf $(bin_top)
	$(VVV) rm -rf $(lib_top)
	$(VVV) rm *.map

distclean: destroy

destroy: mrproper

-include $(deps)

