# umm_malloc (custom heap allocation)

COMPONENT_SUBMODULES	:= umm_malloc
COMPONENT_SRCDIRS		:=
COMPONENT_SRCFILES		:= heap.c umm_malloc/src/umm_malloc.c
COMPONENT_INCDIRS		:= umm_malloc/src umm_malloc/includes/c-helper-macros

COMPONENT_DEPENDS		:= esp8266

LIBMAIN					= mainmm
LIBMAIN_SRC				= $(COMPONENT_LIBDIR)/libmainmm.a
COMPONENT_TARGETS		+= $(LIBMAIN_SRC)

COMPONENT_VARS			+= UMM_POISON_CHECK
ifeq ($(UMM_POISON_CHECK),1)
GLOBAL_CFLAGS			+= -DUMM_POISON_CHECK
endif

# Make copy of libmain and remove mem_manager.o module
$(COMPONENT_RULE)$(LIBMAIN_SRC): $(SDK_LIBDIR)/libmain.a
	$(info Enabling custom heap implementation)
	$(Q) cp $^ $@
	$(Q) $(AR) -d $@ mem_manager.o
