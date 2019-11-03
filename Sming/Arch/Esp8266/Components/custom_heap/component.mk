# umm_malloc (custom heap allocation)

COMPONENT_SUBMODULES	:= umm_malloc
COMPONENT_SRCDIRS		:=
COMPONENT_SRCFILES		:= heap.c umm_malloc/src/umm_malloc.c
COMPONENT_INCDIRS		:= umm_malloc/src umm_malloc/includes/c-helper-macros

COMPONENT_DEPENDS		:= esp8266

COMPONENT_VARS			+= UMM_POISON_CHECK
ifeq ($(UMM_POISON_CHECK),1)
GLOBAL_CFLAGS			+= -DUMM_POISON_CHECK
endif

# remove mem_manager.o module from libmain of SDK
define LIBMAIN_COMMANDS +=
@echo Enabling custom heap implementation
$(Q) $(AR) -d $@ mem_manager.o

endef
