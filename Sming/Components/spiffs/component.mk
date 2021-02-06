## SPIFFS library
COMPONENT_SUBMODULES	:= spiffs
COMPONENT_SRCDIRS		:= . spiffs/src
COMPONENT_INCDIRS		:= . spiffs/src
COMPONENT_DOXYGEN_INPUT	:= spiffs/src

## Spiffy tool

SPIFFY				:= $(TOOLS_BASE)/spiffy$(TOOL_EXT)
COMPONENT_TARGETS	+= $(SPIFFY)
$(COMPONENT_RULE)$(SPIFFY):
	$(call MakeTarget,spiffy/Makefile)


## Application

ifdef DISABLE_SPIFFS
$(error DISABLE_SPIFFS is no longer supported; please remove this option from your component.mk file)
endif

CACHE_VARS			+= SPIFF_FILES SPIFF_BIN
SPIFF_FILES			?= files
SPIFF_BIN			?= spiff_rom
SPIFF_BIN_OUT		:= $(FW_BASE)/$(SPIFF_BIN).bin

COMPONENT_RELINK_VARS	+= SPIFF_FILEDESC_COUNT
SPIFF_FILEDESC_COUNT	?= 7
COMPONENT_CFLAGS		+= -DSPIFF_FILEDESC_COUNT=$(SPIFF_FILEDESC_COUNT)

COMPONENT_CFLAGS		+= -Wno-tautological-compare

##@Building

.PHONY: spiffs-image-update
spiffs-image-update: spiffs-image-clean $(SPIFF_BIN_OUT) ##Rebuild the SPIFFS filesystem image

# Target invoked via partition table
ifneq (,$(filter spiffsgen,$(MAKECMDGOALS)))
PART_TARGET := $(PARTITION_$(PART)_FILENAME)
$(eval PART_FILES := $(call HwExpr,part.build['files']))
SPIFFY_ARGS := $(PARTITION_$(PART)_SIZE_BYTES) "$(or $(PART_FILES),dummy.dir)"
.PHONY: spiffsgen
spiffsgen: $(SPIFFY)
ifneq (,$(PART_TARGET))
	$(Q) mkdir -p $(dir $(PART_TARGET))
	$(Q) $(SPIFFY) $(SPIFFY_ARGS) $(PART_TARGET)
endif
endif
