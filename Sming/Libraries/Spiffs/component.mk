## SPIFFS library
COMPONENT_SUBMODULES	:= spiffs
COMPONENT_SRCDIRS		:= src spiffs/src
COMPONENT_INCDIRS		:= src/include
COMPONENT_DOXYGEN_INPUT	:= spiffs/src

## Application

ifdef DISABLE_SPIFFS
$(error DISABLE_SPIFFS is no longer supported; please remove this option from your component.mk file)
endif

CACHE_VARS			+= SPIFF_FILES SPIFF_BIN
SPIFF_FILES			?= files
SPIFF_BIN			?= spiff_rom

DEBUG_VARS			+= SPIFF_BIN_OUT
SPIFF_BIN_OUT		:= $(FW_BASE)/$(SPIFF_BIN).bin

COMPONENT_RELINK_VARS	+= SPIFF_FILEDESC_COUNT
SPIFF_FILEDESC_COUNT	?= 7
COMPONENT_CXXFLAGS		+= -DSPIFF_FILEDESC_COUNT=$(SPIFF_FILEDESC_COUNT)

COMPONENT_CFLAGS		+= -Wno-tautological-compare

COMPONENT_RELINK_VARS	+= SPIFFS_OBJ_META_LEN
SPIFFS_OBJ_META_LEN		?= 16
COMPONENT_CFLAGS		+= -DSPIFFS_OBJ_META_LEN=$(SPIFFS_OBJ_META_LEN)
COMPONENT_CXXFLAGS		+= -DSPIFFS_OBJ_META_LEN=$(SPIFFS_OBJ_META_LEN)

##@Building

# Spiffs image generation tool
SPIFFSGEN := $(PYTHON) $(COMPONENT_PATH)/spiffsgen.py
SPIFFSGEN_SMING = $(SPIFFSGEN) --meta-len=$(SPIFFS_OBJ_META_LEN) --block-size=8192

HWCONFIG_BUILDSPECS += $(COMPONENT_PATH)/build.json

# Target invoked via partition table
ifneq (,$(filter spiffsgen,$(MAKECMDGOALS)))
PART_TARGET := $(PARTITION_$(PART)_FILENAME)
$(eval PART_FILES := $(call HwExpr,part.build['files']))
.PHONY: spiffsgen
spiffsgen:
ifneq (,$(PART_TARGET))
	@echo "Creating SPIFFS image '$(PART_TARGET)'"
	$(Q) mkdir -p $(dir $(PART_TARGET))
	$(Q) $(SPIFFSGEN_SMING) $(PARTITION_$(PART)_SIZE_BYTES) "$(or $(PART_FILES),)" $(PART_TARGET)
endif
endif
