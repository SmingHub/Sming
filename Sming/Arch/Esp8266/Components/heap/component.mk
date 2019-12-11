COMPONENT_SRCDIRS		:=
COMPONENT_INCDIRS		:=
COMPONENT_SRCFILES		:= heap.c alloc.cpp

# => Custom heap
COMPONENT_VARS			:= ENABLE_CUSTOM_HEAP
ENABLE_CUSTOM_HEAP		?= 0
ifeq ($(ENABLE_CUSTOM_HEAP), 1)
COMPONENT_SUBMODULES	:= umm_malloc
COMPONENT_SRCFILES		+= custom_heap.c umm_malloc/src/umm_malloc.c
COMPONENT_INCDIRS		+= umm_malloc/src umm_malloc/includes/c-helper-macros

#
COMPONENT_VARS			+= UMM_POISON_CHECK
ifeq ($(UMM_POISON_CHECK),1)
GLOBAL_CFLAGS			+= -DUMM_POISON_CHECK
endif

COMPONENT_VARS			+= UMM_FUNC_IRAM
UMM_FUNC_IRAM			?= 1
ifeq ($(UMM_FUNC_IRAM),1)
COMPONENT_CFLAGS		+= -DUMM_FUNC_IRAM=1
endif

COMPONENT_CFLAGS		+= -Wno-array-bounds

# remove mem_manager.o module from libmain of SDK
define HEAP_LIBMAIN_COMMANDS
@echo Enabling custom heap implementation
$(Q) $(AR) -d $@ mem_manager.o

endef
LIBMAIN_COMMANDS += $(HEAP_LIBMAIN_COMMANDS)
endif
