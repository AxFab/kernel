
# =======================================================
# Makefile C
#     Fabien B. <fabien.bavent@gmail.com>
#
# This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# =======================================================


# -------------------------------------------------------
# Programs
CC = gcc
CXX = g++
LD = gcc
AR = ar rc

# -------------------------------------------------------
# Flags building
CFLAGS  = -Wall -Wextra
CFLAGS += -Iinclude/
LFLAGS  =

CFLAGS_debug = $(CFLAGS) -ggdb3
CFLAGS_release = $(CFLAGS) -O3
CFLAGS_testing = $(CFLAGS) -ggdb3 -coverage

LFLAGS_debug = $(LFLAGS)
LFLAGS_release = $(LFLAGS)
LFLAGS_testing = $(LFLAGS) -coverage


# -------------------------------------------------------
# Extract deliveries
FRAGMENTS = $(patsubst src/%,%,$(wildcard src/*))
BIN =
LIB =
SBIN =
SLIB =
TEST = 
DEPS = 

# -------------------------------------------------------
# Create report
ifeq ($(shell which cppcheck > /dev/null || echo N),)
REPORT_CPPCHECK = $(patsubst %,report_cppcheck_%.xml,$(FRAGMENTS))
else 
$(warning 'Cppcheck is not installed')
endif

ifeq ($(shell which rats > /dev/null || echo N),)
REPORT_RATS = $(patsubst %,report_rats_%.xml,$(FRAGMENTS))
else 
$(warning 'Rats is not installed')
endif

ifeq ($(shell pkg-config check || echo N),)
REPORT_CHECK = $(patsubst %,report_check_%.xml,$(TEST))
else 
$(warning 'Check is not installed, will not build any tests')
endif

ifeq ($(shell which gcov > /dev/null || echo N),)
REPORT_GCOV = $(patsubst %,report_gcov_%.xml,$(TEST))
COV_HTML = $(patsubst %,cov_%,$(TEST))
else 
$(warning 'Gcov is not installed')
endif

ifeq ($(shell which valgrind > /dev/null || echo N),)
REPORT_VALGRIND = $(patsubst %,report_valgrind_%.xml,$(TEST))
else 
$(warning 'Gcov is not installed')
endif

REPORTS  = ${REPORT_GCOV} ${REPORT_VALGRIND} ${REPORT_CPPCHECK}
REPORTS += ${REPORT_RATS} ${REPORT_CHECK}

BIN_UT = $(patsubst %,test/%,$(TEST))
	

# -------------------------------------------------------
# Verbose level
V1 = @
V2 = @
V3 = @

E1 = @ true
E2 = @ true
E3 = @ true


# =======================================================
#      Define deliveries
# =======================================================

all: deliveries 

-include deliveries.mk

# -------------------------------------------------------
deliveries: $(BUILD) $(DEVEL) $(REPORTS)

staticreport: ${REPORT_CPPCHECK}

# =======================================================
#      Compile sources files
# =======================================================

# -------------------------------------------------------
# Compile CPP sources
obj/debug/%.o: src/%.cpp obj/debug/%.d
	$(E1) "Compile CPP source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CXX) -c -o $@ $(CFLAGS_debug) $<

obj/debug/%.d: src/%.cpp
	$(E2) "Create CPP dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CXX) -MM -o $@ $(CFLAGS_debug) $<

obj/release/%.o: src/%.cpp obj/release/%.d
	$(E1) "Compile CPP source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CXX) -c -o $@ $(CFLAGS_release) $<

obj/release/%.d: src/%.cpp
	$(E2) echo "Create CPP dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CXX) -MM -o $@ $(CFLAGS_release) $<

obj/testing/%.o: src/%.cpp obj/testing/%.d
	$(E1) echo "Compile CPP source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CXX) -c -o $@ $(CFLAGS_testing) $< \
		`pkg-config check --cflags-only-I --cflags-only-other` 

obj/testing/%.d: src/%.cpp
	$(E2) echo "Create CPP dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CXX) -MM -o $@ $(CFLAGS_testing) $< \
		`pkg-config check --cflags-only-I --cflags-only-other` 

# -------------------------------------------------------
# Compile C sources
obj/debug/%.o: src/%.c obj/debug/%.d
	$(E1) echo "Compile C source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CC) -c -o $@ $(CFLAGS_debug) $<

obj/debug/%.d: src/%.c
	$(E2) echo "Create C dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CC) -MM -o $@ $(CFLAGS_debug) $<

obj/release/%.o: src/%.c obj/release/%.d
	$(E1) echo "Compile C source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CC) -c -o $@ $(CFLAGS_release) $<

obj/release/%.d: src/%.c
	@ echo "Create Compile dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CC) -MM -o $@ $(CFLAGS_release) $<

obj/testing/%.o: src/%.c obj/testing/%.d
	$(E1) echo "Compile C source $@"
	@ mkdir -p $(dir $@)
	$(V1) $(CC) -c -o $@ $(CFLAGS_testing) $< \
		`pkg-config check --cflags-only-I --cflags-only-other` 

obj/testing/%.d: src/%.c
	$(E2) echo "Create C dependencies $@"
	@ mkdir -p $(dir $@)
	$(V2) $(CC) -MM -o $@ $(CFLAGS_testing) $< \
		`pkg-config check --cflags-only-I --cflags-only-other` 


# =======================================================
#      Link objects files
# =======================================================


slib/%.so: $(OBJS_%)
	$(E1) echo "Linking static library $@"
	@ mkdir -p $(dir $@)
	$(V1) $(AR) $@ $^

lib/%.so: $(OBJS_%)
	$(E1) echo "Linking shared library $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -shared -o $@ $(LFLAGS_debug) $^

build/lib/%.so: $(OBJS_%)
	$(E1) echo "Linking shared library $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -shared -o $@ $(LFLAGS_release) $^

bin/%: $(OBJS_%)
	$(E1) echo "Linking program $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -o $@ $(LFLAGS_debug) $^

build/bin/%: $(OBJS_%)
	$(E1) echo "Linking program $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -o $@ $(LFLAGS_release) $^

sbin/%: $(OBJS_%)
	$(E1) echo "Linking program $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -o $@ $(LFLAGS_debug) $^

build/sbin/%: $(OBJS_%)
	$(E1) echo "Linking program $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -o $@ $(LFLAGS_release) $^


test/%: 
	$(E1) echo "Linking program $@"
	@ mkdir -p $(dir $@)
	$(V1) $(LD) -o $@ $(LFLAGS_testing) $(LFLAGS_$(notdir $@)) \
		$(OBJS_$(notdir $@)) `pkg-config --libs check`


# =======================================================
#      Specials targets
# =======================================================

TARS  = $(PACKAGE)-build-$(BUILD).tar.gz 
TARS += $(PACKAGE)-devel-$(BUILD).tar.gz 
TARS += $(PACKAGE)-reports-$(BUILD).tar.gz 

destroy: clean
	@ rm -rf bin lib sbin slib build *.lcov cov_* *.xml test *.tar.gz

clean:
	@ rm -rf obj

coverage: $(COV_HTML)

check: $(REPORT_CHECK)

sonar: $(REPORTS)
	@ sonnar-runner.sh

everything: coverage sonar package

package: $(TARS)

$(PACKAGE)-build-$(VERSION).tar.gz: $(BUILD)
	@ tar czf $@ -C build .

$(PACKAGE)-devel-$(VERSION).tar.gz: $(DEVEL)
	@ tar czf $@ bin lib sbin slib test include

$(PACKAGE)-reports-$(VERSION).tar.gz: $(REPORTS)
	@ tar czf $@ report*

none_:
-include $(DEPS)


# =======================================================
#      Code coverage HTML
# =======================================================

SED_LCOV  = -e '/SF:\/usr.*/,/end_of_record/d'
SED_LCOV += -e '/SF:.*\/src\/tests\/.*/,/end_of_record/d'

# -----------------------------	--------------------------
# Create coverage HTML report
%.lcov: ./test/%
	@ find -name *.gcda | xargs -r rm
	@ CK_FORK=no $<
	@ lcov -c --directory . -b . -o $@
	@ sed $(SED_LCOV) -i $@

cov_%: %.lcov
	@ genhtml -o $@ $<


# =======================================================
#      Code quality reports
# =======================================================

# -------------------------------------------------------
# Static report - depend of sources
report_cppcheck_%.xml: src/%
	@ cppcheck -v --enable=all --xml-version=2 -Iinclude $(wildcard $</*) 2> $@

report_rats_%.xml: src/%
	@ rats -w 3 --xml $(wildcard $</*) > $@


# -------------------------------------------------------
# Dynamic report - depend of tests
report_check_%.xml: test/%
	@ $< -xml

report_gcov_%.xml: test/%
	@ find -name *.gcda | xargs -r rm
	@ CK_FORK=no $<
	@ gcovr -x -r . > $@

report_valgrind_%.xml: test/%
	@ CK_FORK=no valgrind --xml=yes --xml-file=$@  $<


# -------------------------------------------------------
# -------------------------------------------------------
