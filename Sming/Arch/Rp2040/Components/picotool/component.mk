COMPONENT_SUBMODULES := picotool
PICOTOOL_SRC	:= $(COMPONENT_PATH)/picotool
LIBUSB_PATH		:= $(COMPONENT_PATH)/libusb
LIBUSB_DLL		:= libusb-1.0.dll
PICOTOOL		:= $(TOOLS_BASE)/picotool$(TOOL_EXT)
COMPONENT_LIBNAME :=

DEBUG_VARS		+= PICOTOOL

PICOTOOL_CMAKE_OPTIONS :=

ifeq ($(ENABLE_CCACHE),1)
PICOTOOL_CMAKE_OPTIONS += \
	-DCMAKE_C_COMPILER_LAUNCHER=$(CCACHE) \
	-DCMAKE_CPP_COMPILER_LAUNCHER=$(CCACHE)
endif

ifeq ($(UNAME),Windows)
PICOTOOL_CMAKE_OPTIONS += \
	-DLIBUSB_INCLUDE_DIR=$(LIBUSB_PATH) \
	-DLIBUSB_LIBRARIES=$(LIBUSB_PATH)/$(LIBUSB_DLL) \
	-G "MSYS Makefiles"

COMPONENT_TARGETS += $(TOOLS_BASE)/$(LIBUSB_DLL)

$(COMPONENT_RULE)$(TOOLS_BASE)/$(LIBUSB_DLL): $(LIBUSB_PATH)/$(LIBUSB_DLL)
	cp $< $@
endif

COMPONENT_TARGETS += $(PICOTOOL)

$(COMPONENT_RULE)$(PICOTOOL):
	$(Q) mkdir -p $(@D)
	$(Q) cd $(@D) && $(CMAKE) $(PICOTOOL_CMAKE_OPTIONS) $(PICOTOOL_SRC) && $(MAKE)

##@Flashing

.PHONY: picotool
picotool: ##Pass options to picotool, e.g. `make picotool -- help`
	$(Q) $(PICOTOOL) $(CMD)

comma := ,
XIP_BASE := 0x10000000

define CalcHex
$$(printf "0x%x" $$(( $1 )))
endef

define RangeStart
$(call CalcHex,$(XIP_BASE) + $(firstword $(subst $(comma), ,$1)))
endef

define RangeEnd
$(call CalcHex,$(XIP_BASE) + $(firstword $(subst $(comma), ,$1)) + $(word 2,$(subst $(comma), ,$1)))
endef

# Read flash memory into file
# $1 -> `Offset,Size` chunk
# $2 -> Output filename
define ReadFlash
	$(info ReadFlash $1,$2)
	$(Q) $(PICOTOOL) save -r $(call RangeStart,$1) $(call RangeEnd,$1) $2 -t bin
endef

# Read flash manufacturer ID and determine actual size
define ReadFlashID
	$(info ReadFlashID)
	$(Q) $(PICOTOOL) info -a
endef

# Erase a region of Flash
# $1 -> Offset,Size
define EraseFlashRegion
	$(info EraseFlashRegion $1)
	$(Q) $(PICOTOOL) erase -r $(call RangeStart,$1) $(call RangeEnd,$1)
endef

# Erase flash memory contents
define EraseFlash
	$(Q) $(PICOTOOL) erase -a
endef
