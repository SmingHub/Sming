COMPONENT_INCDIRS	:= src/include
COMPONENT_SRCDIRS	:= src
COMPONENT_DOXYGEN_INPUT := src/include

CONFIG_VARS			+= HWCONFIG
ifndef HWCONFIG
override HWCONFIG	:= standard
$(info Using configuration '$(HWCONFIG)')
endif

HWCONFIG_DIRS		:= $(PROJECT_DIR) $(COMPONENT_SEARCH_DIRS) $(ARCH_BASE) $(SMING_HOME)
HWCONFIG_PATH		:= $(firstword $(foreach d,$(HWCONFIG_DIRS),$(wildcard $d/$(HWCONFIG).hw)))
ALL_HWCONFIG		:= $(sort $(notdir $(foreach d,$(HWCONFIG_DIRS),$(wildcard $d/*.hw))))
ALL_HWCONFIG		:= $(ALL_HWCONFIG:.hw=)

ifeq (,$(wildcard $(HWCONFIG_PATH)))
$(info $(HWCONFIG_DIRS))
$(eval $(call PrintVariable,ALL_HWCONFIG))
$(error Hardware configuration '$(HWCONFIG)' not found)
endif

PARTITION_PATH		:= $(COMPONENT_PATH)
PARTITION_TOOLS		:= $(PARTITION_PATH)/Tools
HWCONFIG_TOOL := \
	HWCONFIG_DIRS="$(HWCONFIG_DIRS)" \
	BUILD_BASE=$(BUILD_BASE) \
	$(PYTHON) $(PARTITION_TOOLS)/hwconfig/hwconfig.py

ifeq (,$(MAKE_DOCS))

HWEXPR := $(HWCONFIG_TOOL) $(if $(PART),--part $(PART)) expr $(HWCONFIG) -

define HwExpr
$(shell $(HWEXPR) "$1")
endef

# Obtain partition information
# $1 -> Partition name
# $2 -> Expression
define PartInfo
$(shell $(HWCONFIG_TOOL) --part $1 expr $(HWCONFIG) - $2)
endef


# Import PARTITION_TABLE_OFFSET from hardware configuration
DEBUG_VARS += PARTITION_TABLE_OFFSET
SMING_ARCH_HW := $(call HwExpr,config.arch)
ifneq ($(SMING_ARCH),$(SMING_ARCH_HW))
$(error Hardware configuration '$(HWCONFIG)' is for arch $(SMING_ARCH_HW), does not match SMING_ARCH ($(SMING_ARCH)))
endif
PARTITION_TABLE_OFFSET	:= $(call HwExpr,(config.partitions.offset_str()))
COMPONENT_CXXFLAGS		:= -DPARTITION_TABLE_OFFSET=$(PARTITION_TABLE_OFFSET)


##@Configuration

.PHONY: map
map: $(HWCONFIG_PATH) ##Show memory map
	$(Q) $(HWEXPR) "config.map().to_csv()"

.PHONY: hwexpr
hwexpr: $(HWCONFIG_PATH) ##Evaluate expression against hardware configuration (use EXPR= and optionally PART=)
	$(Q) $(HWEXPR) "$(EXPR)"

.PHONY: hwconfig
hwconfig: $(HWCONFIG_PATH) ##Show current hardware configuration
	@echo
	@echo Hardware Configuration
	@echo Available: $(ALL_HWCONFIG)
	@echo Selected: $(HWCONFIG)
	$(Q) $(HWEXPR) "config.to_json()"
	@echo

.PHONY: hwconfig-create
hwconfig-create: ##Create hardware configuration from current build variables
	$(Q) $(HWCONFIG_TOOL) create-config $(CONFIG_CACHE_FILE) -


##@Building

# The partition table
PARTITIONS_BIN := $(FW_BASE)/partitions.bin
CUSTOM_TARGETS += $(PARTITIONS_BIN)

$(PARTITIONS_BIN): $(HWCONFIG_PATH)
	$(Q) $(HWCONFIG_TOOL) partgen $(HWCONFIG) $@


##@Flashing

# $1 -> Filter expression
define PartChunks
$(call HwExpr,(' '.join(['%s=%s' % (part.address_str(), part.filename) for part in filter(lambda part: $1 and part.filename != '', config.partitions)])))
endef

# One flash sector of 0xFF
BLANK_BIN := $(PARTITION_PATH)/blank.bin

DEBUG_VARS += FLASH_APP_CHUNKS FLASH_PARTITION_CHUNKS
$(eval FLASH_APP_CHUNKS = $(call PartChunks,(part.type_is('app'))))
$(eval FLASH_PARTITION_CHUNKS = $(PARTITION_TABLE_OFFSET)=$(PARTITIONS_BIN) $(call PartChunks,True))

ifdef PART
DEBUG_VARS += FLASH_PART_CHUNKS
$(eval FLASH_PART_CHUNKS = $(call PartChunks,(part=='$(PART)')))
endif


endif # MAKE_DOCS
