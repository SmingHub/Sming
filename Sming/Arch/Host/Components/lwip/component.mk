ifeq ($(SMING_RELEASE),1)
	CFLAGS += -DLWIP_NOASSERT
endif


# => LWIP
ENABLE_CUSTOM_LWIP ?= 2
ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	LWIP_BASE			:= $(ARCH_COMPONENTS)/lwip
	SUBMODULES			+= $(LWIP_BASE)/lwip
	EXTRA_INCDIR		+= $(LWIP_BASE) $(LWIP_BASE)/lwip/src/include
	LIBLWIP				:= lwip
	LIBS				+= $(LIBLWIP)
	ENABLE_LWIPDEBUG	?= 0
	ifeq ($(ENABLE_LWIPDEBUG), 1)
		CMAKE_OPTIONS	:= -DCMAKE_BUILD_TYPE=Debug
	else
		CMAKE_OPTIONS	:= -DCMAKE_BUILD_TYPE=Release
	endif
	LWIP_BUILD_DIR		:= $(BUILD_BASE)/$(LWIP_BASE)

	ifeq ($(UNAME),Windows)
		EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/win32/include
		CMAKE_OPTIONS	+= -G "MSYS Makefiles"
	else
		EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/unix/port/include
	endif

	CLEAN				+= lwip-clean

$(call UserLibPath,lwip): $(LWIP_BUILD_DIR)/Makefile
	$(vecho) "Building $(notdir $@)..."
	$(Q) $(MAKE) -C $(LWIP_BUILD_DIR)

$(LWIP_BUILD_DIR)/Makefile: $(LWIP_BASE)/lwip/.submodule | $(LWIP_BUILD_DIR)
	$(Q) cd $(LWIP_BUILD_DIR); \
	$(CMAKE) -DUSER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)" $(CMAKE_OPTIONS) $(SMING_HOME$)/$(LWIP_BASE)/$(UNAME)

$(LWIP_BUILD_DIR):
	mkdir -p $@

.PHONY: lwip-clean
lwip-clean:
	-$(Q) rm -rf $(LWIP_BUILD_DIR)
endif




# APP




# => LWIP
LWIP_BASE		:= $(ARCH_COMPONENTS)/lwip
LIBS			+= lwip
EXTRA_INCDIR	+= $(LWIP_BASE) $(LWIP_BASE)/lwip/src/include
ifeq ($(UNAME),Windows)
EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/win32/include
else
EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/unix/port/include
endif

