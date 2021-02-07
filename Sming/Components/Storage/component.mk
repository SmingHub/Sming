COMPONENT_INCDIRS	:= src/include
COMPONENT_SRCDIRS	:= src
COMPONENT_DOXYGEN_INPUT := src/include

CONFIG_VARS			+= HWCONFIG
ifndef HWCONFIG
override HWCONFIG	:= standard
$(info Using configuration '$(HWCONFIG)')
endif

# Directories to search for hardware config
HWCONFIG_DIRS		:= $(PROJECT_DIR) $(COMPONENT_SEARCH_DIRS) $(ARCH_BASE) $(SMING_HOME)

# List of all hardware configurations
ALL_HWCONFIG_PATHS	:= $(foreach d,$(HWCONFIG_DIRS),$(wildcard $d/*.hw))
ALL_HWCONFIG		:= $(sort $(notdir $(ALL_HWCONFIG_PATHS)))
ALL_HWCONFIG		:= $(ALL_HWCONFIG:.hw=)

# Path to selected hardware config file
HWCONFIG_PATH		:= $(firstword $(foreach d,$(HWCONFIG_DIRS),$(wildcard $d/$(HWCONFIG).hw)))

ifeq (,$(wildcard $(HWCONFIG_PATH)))
ifeq (,$(MAKE_CLEAN))
$(info $(HWCONFIG_DIRS))
$(eval $(call PrintVariable,ALL_HWCONFIG,Available configurations))
$(error Hardware configuration '$(HWCONFIG)' not found)
endif
endif

PARTITION_PATH		:= $(COMPONENT_PATH)
PARTITION_TOOLS		:= $(PARTITION_PATH)/Tools
HWCONFIG_TOOL := \
	HWCONFIG_DIRS="$(HWCONFIG_DIRS)" \
	BUILD_BASE=$(BUILD_BASE) \
	$(PYTHON) $(PARTITION_TOOLS)/hwconfig/hwconfig.py

ifeq (,$(MAKE_DOCS))

# Generate build variables from hardware configuration
HWCONFIG_MK := $(PROJECT_DIR)/$(OUT_BASE)/hwconfig.mk
$(shell $(HWCONFIG_TOOL) --quiet expr $(HWCONFIG) $(HWCONFIG_MK) "config.buildVars()")
include $(HWCONFIG_MK)

HWEXPR := $(HWCONFIG_TOOL) $(if $(PART),--part $(PART)) expr $(HWCONFIG) -

define HwExpr
$(shell $(HWEXPR) "$1")
endef

# Import PARTITION_TABLE_OFFSET from hardware configuration
DEBUG_VARS += PARTITION_TABLE_OFFSET
ifeq ($(SMING_ARCH_HW),)
$(error Hardware configuration error)
else ifneq ($(SMING_ARCH),$(SMING_ARCH_HW))
$(error Hardware configuration is for arch $(SMING_ARCH_HW), does not match SMING_ARCH ($(SMING_ARCH)))
endif
COMPONENT_CXXFLAGS		:= -DPARTITION_TABLE_OFFSET=$(PARTITION_TABLE_OFFSET)


##@Configuration

.PHONY: map
map: $(HWCONFIG_PATH) ##Show partition map
	@echo "Partition map:"
	$(Q) $(HWEXPR) "config.map().to_csv()"
	@echo

.PHONY: hwexpr
hwexpr: $(HWCONFIG_PATH) ##Evaluate expression against hardware configuration (use EXPR= and optionally PART=)
	$(Q) $(HWEXPR) "$(EXPR)"

.PHONY: hwconfig
hwconfig: $(HWCONFIG_PATH) ##Show current hardware configuration
	@echo
	$(Q) $(HWEXPR) "config.to_json()"
	@echo
ifneq ($(V),)
	@echo "$(HWCONFIG): $(foreach c,$(HWCONFIG_DEPENDS),\n  $c)"
endif

.PHONY: hwconfig-list
hwconfig-list: ##List available hardware configurations
	@echo "Available configurations: $(foreach c,$(ALL_HWCONFIG),\n $(if $(subst $c,,$(HWCONFIG)), ,*) $(shell printf "%-25s" "$c") $(filter %/$c.hw,$(ALL_HWCONFIG_PATHS)))"
	@echo

.PHONY: hwconfig-validate
hwconfig-validate: $(HWCONFIG_PATH) ##Validate current hardware configuration
	@echo "Validating hardware config '$(HWCONFIG)'"
	$(Q) $(HWCONFIG_TOOL) validate $(HWCONFIG) - $(PARTITION_PATH)/schema.json


##@Building

# The partition table
PARTITIONS_BIN := $(FW_BASE)/partitions.bin
CUSTOM_TARGETS += $(PARTITIONS_BIN)

$(PARTITIONS_BIN): $(HWCONFIG_DEPENDS)
	$(Q) $(MAKE) --no-print-directory hwconfig-validate
	$(Q) $(HWCONFIG_TOOL) partgen $(HWCONFIG) $@


# Create build target for a partition
# $1 -> Partition name
define PartitionTarget
PTARG := $(shell $(HWCONFIG_TOOL) --part $1 expr $(HWCONFIG) - part.filename)
$$(PTARG):
	$$(Q) $$(MAKE) --no-print-directory $$(shell $$(HWCONFIG_TOOL) --part $1 expr $$(HWCONFIG) - "part.build['target']") PART=$1
CUSTOM_TARGETS += $$(PTARG)
endef

# Create build targets for all partitions with 'build' property
DEBUG_VARS += PARTITIONS_WITH_TARGETS
PARTITIONS_WITH_TARGETS := $(call HwExpr,(' '.join([part.name for part in filter(lambda part: part.build is not None, config.partitions)])))

# Must be invoked from project.mk after all Components have been processed
# This allows partition definitions to include variables which may not yet be defined
define PartitionCreateTargets
$(foreach p,$(PARTITIONS_WITH_TARGETS),$(eval $(call PartitionTarget,$p)))
endef


##@Flashing

# $1 -> Partition name
define PartChunk
$(if $(PARTITION_$1_FILENAME),$(PARTITION_$1_ADDRESS)=$(PARTITION_$1_FILENAME))
endef

# $1 -> Partition name
define PartRegion
$(PARTITION_$1_ADDRESS),$(PARTITION_$1_SIZE)
endef

# One flash sector of 0xFF
BLANK_BIN := $(PARTITION_PATH)/blank.bin

# Just the application chunks
FLASH_APP_CHUNKS = $(foreach p,$(SPIFLASH_PARTITION_NAMES),$(if $(filter app,$(PARTITION_$p_TYPE)),$(call PartChunk,$p)))
# Partition map chunk
FLASH_MAP_CHUNK := $(PARTITION_TABLE_OFFSET)=$(PARTITIONS_BIN)
# All partitions with image files
FLASH_PARTITION_CHUNKS = $(foreach p,$(SPIFLASH_PARTITION_NAMES),$(call PartChunk,$p))

ifdef PART
DEBUG_VARS += FLASH_PART_CHUNKS FLASH_PART_REGION
FLASH_PART_CHUNKS := $(call PartChunk,$(PART))
FLASH_PART_REGION := $(call PartRegion,$(PART))
endif

# Where to store read partition map file
READMAP_BIN := $(OUT_BASE)/partition-table.read.bin
PARTITION_TABLE_REGION = $(PARTITION_TABLE_OFFSET),$(PARTITION_TABLE_LENGTH)

.PHONY: readmap
readmap:##Read partition map from device
	$(Q) $(call ReadFlash,$(PARTITION_TABLE_REGION),$(READMAP_BIN))
	@echo
	@echo "Partition map read from device:"
	$(Q) $(HWCONFIG_TOOL) expr $(READMAP_BIN) - "config.map().to_csv()"
	@echo

.PHONY: flashpart
flashpart: all kill_term ##Flash a specific partition, set PART=name
	$(call WriteFlash,$(FLASH_PART_CHUNKS))

.PHONY: erasepart
erasepart: kill_term ##Erase a specific partition, set PART=name
	$(call EraseFlashRegion,$(FLASH_PART_REGION))

.PHONY: readpart
readpart: kill_term ##Read partition from device, set PART=name
	$(call ReadFlash,$(FLASH_PART_REGION),$(OUT_BASE)/$(PART).read.bin)

.PHONY: flashmap
flashmap: $(PARTITIONS_BIN) kill_term ##Write partition table to device
	$(call WriteFlash,$(FLASH_MAP_CHUNK))


endif # MAKE_DOCS
