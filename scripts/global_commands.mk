
define objs
	$(patsubst src/%.c,$(OBJS_DIR)/$(1)/%.o,    \
	$(patsubst src/%.cpp,$(OBJS_DIR)/$(1)/%.o,  \
	$(patsubst src/%.asm,$(OBJS_DIR)/$(1)/%.o,  \
	$(2)                                        \
	)))
endef

DEPS = $(patsubst %.o,%.d,$(call objs,kernel,$(KRN_SRC)))

style:
	@ astyle $(ASTYLE) $(ALL_SRC) $(ALL_INC)
	@ find -name *.orig | xargs -r rm


sources: 
	@ echo $(ALL_SRC)

deps:
	@ echo $(DEPS)


ifeq ($(MAKECMDGOALS),clean)
NODEPS = 1
endif
ifeq ($(MAKECMDGOALS),destroy)
NODEPS = 1
endif

ifeq ($(NODEPS),)
-include $(DEPS)
endif

