# umm_malloc (custom heap allocation)

COMPONENT_SUBMODULES	:= umm_malloc
COMPONENT_SRCDIRS		:= . umm_malloc/src
COMPONENT_INCDIRS		:= umm_malloc/src umm_malloc/includes/c-helper-macros

LIBMAIN					= mainmm
LIBMAIN_SRC				= $(COMPONENT_LIBDIR)/libmainmm.a
COMPONENT_TARGETS		+= $(LIBMAIN_SRC)

# Make copy of libmain and remove mem_manager.o module
$(COMPONENT_RULE)$(LIBMAIN_SRC): $(SDK_LIBDIR)/libmain.a
	$(info Enabling custom heap implementation)
	$(Q) cp $^ $@
	$(Q) $(AR) -d $@ mem_manager.o
