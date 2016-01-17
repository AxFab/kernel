# Compile assembly for usermode kernel
# ---------------------------------------------------------------------------
ifeq ($(obj_dir),)
$(error  This file is part of a larger script!)
else

# Nothing to do here - but this file must be present

coverage: $(bld_dir)/cov_kImage

# =======================================================
#      Code coverage HTML
# =======================================================
SED_LCOV  = -e '/SF:\/usr.*/,/end_of_record/d'
SED_LCOV += -e '/SF:.*\/src\/tests\/.*/,/end_of_record/d'

# -------------------------------------------------------
# Create coverage HTML report
$(bld_dir)/%.lcov: $(bin_dir)/%
	@ find $(obj_dir) -name *.gcda | xargs -r rm
	@ CK_FORK=no `readlink -f $<`
	@ lcov -c --directory . -b . -o $@
	@ sed $(SED_LCOV) -i $@

$(bld_dir)/cov_%: $(bld_dir)/%.lcov
	@ genhtml -o $@ $<
	

endif
