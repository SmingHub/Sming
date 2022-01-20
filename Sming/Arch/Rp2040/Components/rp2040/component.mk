COMPONENT_SUBMODULES := pico-sdk

ifdef PICO_SDK_PATH
export PICO_SDK_PATH := $(call FixPath,$(PICO_SDK_PATH))
else
export PICO_SDK_PATH := $(COMPONENT_PATH)/pico-sdk
endif

COMPONENT_RELINK_VARS += PICO_BOARD
ifndef PICO_BOARD
export PICO_BOARD=pico
endif

GLOBAL_CFLAGS += \
	-DPICO_ON_DEVICE=1

# Press BOOTSEL to reboot into programming mode
COMPONENT_RELINK_VARS := ENABLE_BOOTSEL
ifndef SMING_RELEASE
ENABLE_BOOTSEL ?= 1
endif
ifeq ($(ENABLE_BOOTSEL),1)
COMPONENT_CXXFLAGS += -DENABLE_BOOTSEL=1
endif

WRAPPED_FUNCTIONS :=

$(foreach c,$(wildcard $(COMPONENT_PATH)/sdk/*.mk),$(eval include $c))

EXTRA_LDFLAGS := \
	$(call Wrap,$(WRAPPED_FUNCTIONS)) \
	-T memmap_default.ld

SDK_INTERFACES := \
	boards \
	common/pico_base \
	common/pico_binary_info \
	common/pico_bit_ops \
	common/pico_divider \
	common/pico_sync \
	common/pico_util \
	rp2040/hardware_regs \
	rp2040/hardware_structs \
	rp2_common/hardware_gpio \
	rp2_common/pico_platform \
	rp2_common/hardware_base \
	rp2_common/hardware_sync \
	rp2_common/hardware_divider \
	rp2_common/hardware_timer \
	rp2_common/hardware_clocks \
	rp2_common/hardware_dma \
	rp2_common/hardware_exception \
	rp2_common/hardware_flash \
	rp2_common/hardware_irq \
	rp2_common/hardware_pio \
	rp2_common/hardware_resets \
	rp2_common/hardware_rosc \
	rp2_common/hardware_rtc \
	rp2_common/hardware_pll \
	rp2_common/hardware_vreg \
	rp2_common/hardware_watchdog \
	rp2_common/hardware_xosc \
	rp2_common/pico_bootrom \
	rp2_common/pico_double \
	rp2_common/pico_int64_ops \
	rp2_common/pico_float \
	rp2_common/pico_runtime \
	rp2_common/pico_unique_id

COMPONENT_INCDIRS := \
	src/include \
	$(foreach p,$(SDK_INTERFACES),$(PICO_SDK_PATH)/src/$p/include)


COMPONENT_SRCDIRS := src

RP2040_COMPONENT_DIR := $(COMPONENT_PATH)
PICO_BUILD_DIR	:= $(COMPONENT_BUILD_BASE)/sdk
PICO_BASE_DIR	:= $(PICO_BUILD_DIR)/generated/pico_base
PICO_CONFIG		:= $(PICO_BASE_DIR)/pico/config_autogen.h
PICO_LIB		:= $(PICO_BUILD_DIR)/libpico.a

COMPONENT_INCDIRS += $(PICO_BASE_DIR)

LIBDIRS += \
	$(PICO_SDK_PATH)/src/rp2_common/pico_standard_link \
	$(PICO_BUILD_DIR)

EXTRA_LIBS += \
	pico \
	m

RP2040_CMAKE_OPTIONS := \
	-G Ninja \
	-DCMAKE_MAKE_PROGRAM=$(NINJA)

COMPONENT_PREREQUISITES := $(PICO_CONFIG)

BOOTLOADER := $(PICO_BUILD_DIR)/pico-sdk/src/rp2_common/boot_stage2/bs2_default_padded_checksummed.S

COMPONENT_TARGETS := \
	$(PICO_LIB)

$(PICO_CONFIG): $(PICO_BUILD_DIR)
	$(Q) cd $(PICO_BUILD_DIR) && $(CMAKE) $(RP2040_CMAKE_OPTIONS) $(RP2040_COMPONENT_DIR)/sdk

$(COMPONENT_RULE)$(PICO_LIB):
	$(Q) cd $(@D) && $(NINJA)

ifdef COMPONENT_RULE
$(PICO_BUILD_DIR):
	$(Q) mkdir -p $@
endif


# Pull in board configuration, specifically PICO_FLASH_SIZE_BYTES
# It's only available in a .h file so needs some parsing
RP2040_BOARD_PATH := $(PICO_SDK_PATH)/src/boards/include/boards
RP2040_BOARD_HEADERS = $(notdir $(wildcard $(RP2040_BOARD_PATH)/*.h))
RP2040_BOARDS = $(RP2040_BOARD_HEADERS:.h=)

# Used to un-escape newlines from AWK script
define NewLine


endef

BOARD_HEADER_PATH := $(PICO_SDK_PATH)/src/boards/include/boards/$(PICO_BOARD).h

ifneq (,$(wildcard $(BOARD_HEADER_PATH)))
$(eval $(subst \n,$(NewLine),$(shell $(AWK) -f $(RP2040_COMPONENT_DIR)/board-info.awk $(BOARD_HEADER_PATH))))
endif

DEBUG_VARS += $(RP2040_BOARD_VARS)


CUSTOM_TARGETS += check-flash-size

.PHONY: check-flash-size
check-flash-size:
	@if [ $$(( $(STORAGE_DEVICE_spiFlash_SIZE_BYTES) - $(PICO_FLASH_SIZE_BYTES) )) -gt 0 ]; then \
		echo; \
		echo "ERROR: Hardware config spiFlash size exceeds value board definition"; \
		echo "PICO_FLASH_SIZE_BYTES = $(PICO_FLASH_SIZE_BYTES)"; \
		echo "spiFlash.size = $(STORAGE_DEVICE_spiFlash_SIZE_BYTES)"; \
		echo; \
		echo "Please fix your hardware configuration!"; \
		echo; \
		exit 1; \
	fi


##@Help

.PHONY: list-boards
list-boards: ##List development boards defined in SDK
	$(call PrintVariable,RP2040_BOARDS)


.PHONY: board-info
board-info: ##List RP2040 board configuration
	$(call PrintVariableRefs,RP2040_BOARD_VARS)
