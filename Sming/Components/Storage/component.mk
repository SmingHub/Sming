COMPONENT_INCDIRS	:= src/include
COMPONENT_SRCDIRS	:= src
COMPONENT_DOXYGEN_INPUT := src/include

CONFIG_VARS			+= HWCONFIG HWCONFIG_OPTS
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
	HWCONFIG_OPTS="$(HWCONFIG_OPTS)" \
	BUILD_BASE=$(BUILD_BASE) \
	$(PYTHON) $(PARTITION_TOOLS)/hwconfig/hwconfig.py

HWCONFIG_MK := $(PROJECT_DIR)/$(OUT_BASE)/hwconfig.mk
ifneq (,$(MAKE_DOCS)$(MAKE_CLEAN))
-include $(HWCONFIG_MK)
else
# Generate build variables from hardware configuration
$(shell $(HWCONFIG_TOOL) --quiet expr $(HWCONFIG) $(HWCONFIG_MK) "config.buildVars()")
include $(HWCONFIG_MK)
ifeq ($(SMING_ARCH_HW),)
$(error Hardware configuration error)
else ifneq ($(SMING_ARCH),$(SMING_ARCH_HW))
$(error Hardware configuration is for arch $(SMING_ARCH_HW), does not match SMING_ARCH ($(SMING_ARCH)))
endif
COMPONENT_CXXFLAGS := -DPARTITION_TABLE_OFFSET=$(PARTITION_TABLE_OFFSET)
COMPONENT_CPPFLAGS := -DPARTITION_TABLE_OFFSET=$(PARTITION_TABLE_OFFSET)

# Function to evaluate expression against config
HWEXPR := $(HWCONFIG_TOOL) $(if $(PART),--part "$(PART)") expr $(HWCONFIG) -

define HwExpr
$(shell $(HWEXPR) "$1")
endef


##@Configuration

.PHONY: map
map: $(HWCONFIG_PATH) ##Show partition map
	@echo "Partition map: $(HWCONFIG)"
	$(Q) $(HWEXPR) "'options: %s\n%s' % (', '.join(config.options), config.map().to_csv())"
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

.PHONY: hwconfig-options
hwconfig-options: ##List available hardware configuration options
	@echo
	@echo "Available options (use with HWCONFIG_OPTS or in custom profile):"
	$(Q) $(HWEXPR) "''.join(('  %-10s %s\n' % (k, v['description']) for k, v in config.option_library.items()))"
	@echo

.PHONY: hwconfig-validate
hwconfig-validate: $(HWCONFIG_PATH) ##Validate current hardware configuration
	@echo "Validating hardware config '$(HWCONFIG)'"
	$(Q) $(HWCONFIG_TOOL) validate $(HWCONFIG) - $(PARTITION_PATH)/schema.json


##@Building

# The partition table
PARTITIONS_BIN := $(FW_BASE)/partitions.bin
CUSTOM_TARGETS += partmap-build

.PHONY: partmap-build
partmap-build:
	$(Q) $(MAKE) --no-print-directory hwconfig-validate
	$(Q) $(HWCONFIG_TOOL) partgen $(HWCONFIG) $(PARTITIONS_BIN)


# Create build target for a partition
# $1 -> Partition name
define PartitionTarget
$(PARTITION_$1_FILENAME):
	$$(Q) $$(MAKE) --no-print-directory $$(shell $$(HWCONFIG_TOOL) --part $1 expr $$(HWCONFIG) - "part.build['target']") PART=$1
CUSTOM_TARGETS += $(PARTITION_$1_FILENAME)
endef

# Must be invoked from project.mk after all Components have been processed
# This allows partition definitions to include variables which may not yet be defined
define PartitionCreateTargets
$(foreach p,$(PARTITIONS_WITH_TARGETS),$(eval $(call PartitionTarget,$p)))
endef

.PHONY: buildpart
buildpart: ##Rebuild all partition targets
ifeq (,$(PARTITION_BUILD_TARGETS))
	@echo "No partitions have build targets"
else
	$(Q) rm -f $(PARTITION_BUILD_TARGETS)
	$(MAKE) $(PARTITION_BUILD_TARGETS)
endif

##@Flashing

# Get chunks for a list of partitions, ignore any without filenames
# $1 -> List of partition names
define PartChunks
$(foreach p,$1,$(if $(PARTITION_$p_FILENAME),$(PARTITION_$p_ADDRESS)=$(PARTITION_$p_FILENAME)))
endef

# Get regions for list of partitions
# $1 -> List of partition names
define PartRegions
$(foreach p,$1,$(PARTITION_$p_ADDRESS),$(PARTITION_$p_SIZE_BYTES))
endef

# One flash sector of 0xFF
BLANK_BIN := $(PARTITION_PATH)/blank.bin

# Just the application chunks
SPIFLASH_APP_PARTITION_NAMES := $(foreach p,$(SPIFLASH_PARTITION_NAMES),$(if $(filter app,$(PARTITION_$p_TYPE)),$p))
FLASH_APP_CHUNKS = $(call PartChunks,$(SPIFLASH_APP_PARTITION_NAMES))
# Partition map chunk
FLASH_MAP_CHUNK := $(PARTITION_TABLE_OFFSET)=$(PARTITIONS_BIN)
# All partitions with image files
FLASH_PARTITION_CHUNKS = $(call PartChunks,$(SPIFLASH_PARTITION_NAMES))

# User-selected partition(s)
FLASH_PART_CHUNKS = $(call PartChunks,$(PART))
FLASH_PART_REGION = $(call PartRegions,$(PART))

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

# Run sanity checks against a list of partitions before flashing commences
# $1 -> List of partition names
define CheckPartitionChunks
$(HWCONFIG_TOOL) flashcheck $(HWCONFIG) - "$1"
endef

.PHONY: flashpart
flashpart: all kill_term ##Flash a specific partition, set PART=name
	$(Q) $(call CheckPartitionChunks,$(FLASH_PART_CHUNKS))
	$(call WriteFlash,$(FLASH_PART_CHUNKS))

.PHONY: erasepart
erasepart: kill_term ##Erase a specific partition, set PART=name
	$(call EraseFlashRegion,$(FLASH_PART_REGION))

.PHONY: readpart
readpart: kill_term ##Read partition from device, set PART=name
	$(call ReadFlash,$(FLASH_PART_REGION),$(OUT_BASE)/$(PART).read.bin)

.PHONY: flashmap
flashmap: partmap-build kill_term ##Write partition table to device
	$(call WriteFlash,$(FLASH_MAP_CHUNK))


endif # MAKE_DOCS

##@Cleaning

clean: part-clean

.PHONY: part-clean
part-clean: ##Clean partition targets
	$(Q) rm -f $(PARTITION_BUILD_TARGETS)
