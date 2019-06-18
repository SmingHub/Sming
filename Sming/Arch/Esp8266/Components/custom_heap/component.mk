# umm_malloc (custom heap allocation)

# => SMING

CONFIG_VARS += ENABLE_CUSTOM_HEAP
ENABLE_CUSTOM_HEAP		?= 0
ifeq ($(ENABLE_CUSTOM_HEAP), 1)
	CUSTOM_HEAP_BASE	:= $(ARCH_COMPONENTS)/custom_heap
	UMM_MALLOC_BASE		:= $(CUSTOM_HEAP_BASE)/umm_malloc
	SUBMODULES			+= $(UMM_MALLOC_BASE)
	MODULES				+= $(CUSTOM_HEAP_BASE) $(UMM_MALLOC_BASE)/src
	EXTRA_INCDIR		+= $(UMM_MALLOC_BASE)/src $(UMM_MALLOC_BASE)/includes/c-helper-macros
	LIBMAINMM			:= mainmm
	LIBS				+= $(LIBMAINMM)

# Make copy of libmain and remove mem_manager.o module
$(call UserLibPath,$(LIBMAINMM)): $(SDK_LIBDIR)/libmain.a | $(UMM_MALLOC_BASE)/.submodule
	$(vecho) "Enabling custom heap implementation"
	$(Q) cp $^ $@
	$(Q) $(AR) -d $@ mem_manager.o
endif


# => APP

CONFIG_VARS += ENABLE_CUSTOM_HEAP
ENABLE_CUSTOM_HEAP ?= 0
ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN			:= mainmm
	LIBMAIN_SRC		:= $(call UserLibPath,$(LIBMAIN))
	CUSTOM_TARGETS	+= $(LIBMAIN_SRC)

$(LIBMAIN_SRC):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_HEAP=1)
else
    LIBMAIN			:= main
	LIBMAIN_SRC		:= $(SDK_LIBDIR)/lib$(LIBMAIN).a
endif

