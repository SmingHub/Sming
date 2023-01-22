COMPONENT_SUBMODULES := pico-sdk

ifdef PICO_SDK_PATH
export PICO_SDK_PATH := $(call FixPath,$(PICO_SDK_PATH))
else
export PICO_SDK_PATH := $(COMPONENT_PATH)/pico-sdk
endif

ifeq ($(DISABLE_WIFI),1)
export PICO_BOARD ?= pico
else
export PICO_BOARD ?= pico_w
COMPONENT_DEPENDS += uzlib
endif

COMPONENT_VARS := PICO_BOARD DISABLE_WIFI DISABLE_NETWORK

PICO_SDK_VARS := PICO_BOARD=$(PICO_BOARD)
PICO_SDK_LIBHASH := $(call CalculateVariantHash,PICO_SDK_VARS)

GLOBAL_CFLAGS += \
	-DPICO_ON_DEVICE=1 \
	-DCYW43_LWIP=1 \
	-DPICO_CYW43_ARCH_POLL=1

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
	common/pico_time \
	common/pico_util \
	rp2040/hardware_regs \
	rp2040/hardware_structs \
	rp2_common/cyw43_driver \
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
	rp2_common/hardware_spi \
	rp2_common/hardware_vreg \
	rp2_common/hardware_watchdog \
	rp2_common/hardware_xosc \
	rp2_common/pico_bootrom \
	rp2_common/pico_double \
	rp2_common/pico_int64_ops \
	rp2_common/pico_float \
	rp2_common/pico_runtime \
	rp2_common/pico_unique_id \
	rp2_common/pico_cyw43_arch

COMPONENT_INCDIRS := \
	src/include \
	$(foreach p,$(SDK_INTERFACES),$(PICO_SDK_PATH)/src/$p/include)


COMPONENT_SRCDIRS := src

RP2040_COMPONENT_DIR := $(COMPONENT_PATH)
PICO_BUILD_DIR	:= $(COMPONENT_BUILD_BASE)/sdk-$(PICO_SDK_LIBHASH)
PICO_BASE_DIR	:= $(PICO_BUILD_DIR)/generated/pico_base
PICO_CONFIG		:= $(PICO_BASE_DIR)/pico/config_autogen.h
PICO_LIB		:= $(PICO_BUILD_DIR)/libpico.a

COMPONENT_INCDIRS += $(PICO_BASE_DIR)

LIBDIRS += \
	$(PICO_SDK_PATH)/src/rp2_common/pico_standard_link \
	$(PICO_BUILD_DIR)

EXTRA_LIBS += \
	pico \
	m \
	stdc++ \
	gcc

RP2040_CMAKE_OPTIONS := \
	-G Ninja \
	-DCMAKE_MAKE_PROGRAM=$(NINJA)

COMPONENT_PREREQUISITES := $(PICO_CONFIG)

BOOTLOADER := $(PICO_BUILD_DIR)/pico-sdk/src/rp2_common/boot_stage2/bs2_default_padded_checksummed.S

DEBUG_VARS += CYW43_FIRMWARE
CYW43_FIRMWARE := $(COMPONENT_BUILD_BASE)/cyw43-fw.gz

COMPONENT_RELINK_VARS += LINK_CYW43_FIRMWARE
LINK_CYW43_FIRMWARE ?= 1
ifeq ($(LINK_CYW43_FIRMWARE),1)
COMPONENT_CPPFLAGS += -DCYW43_FIRMWARE=\"$(CYW43_FIRMWARE)\"
endif

COMPONENT_TARGETS := \
	$(PICO_LIB) \
	$(CYW43_FIRMWARE)

$(PICO_CONFIG): $(PICO_BUILD_DIR) $(PICO_SDK_PATH)/lib/cyw43-driver.patch $(PICO_SDK_PATH)/lib/cyw43-driver/.submodule $(PICO_SDK_PATH)/lib/lwip/.submodule
	$(Q) cd $(PICO_BUILD_DIR) && $(CMAKE) $(RP2040_CMAKE_OPTIONS) $(RP2040_COMPONENT_DIR)/sdk

$(COMPONENT_RULE)$(PICO_LIB):
	$(Q) cd $(@D) && $(NINJA)

$(PICO_SDK_PATH)/lib/cyw43-driver.patch: $(RP2040_COMPONENT_DIR)/cyw43-driver.patch
	cp $< $@

$(COMPONENT_RULE)$(CYW43_FIRMWARE):
	@echo ">>> Creating CYW43 Firmware BLOB ..."
	$(Q) cd "$(RP2040_COMPONENT_DIR)/firmware" && ./build.sh $@
	@echo "Created $@"

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
