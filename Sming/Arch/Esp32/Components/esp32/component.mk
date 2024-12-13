SDK_COMPONENTS_PATH := $(IDF_PATH)/components

COMPONENT_DEPENDS := libc

COMPONENT_SRCDIRS := src
COMPONENT_INCDIRS := src/include include

# Applications can provide file with custom SDK configuration settings
CACHE_VARS += SDK_CUSTOM_CONFIG

COMPONENT_RELINK_VARS += DISABLE_NETWORK DISABLE_WIFI

ifneq (,$(filter v4.%,$(IDF_VERSION)))
IDF_VERSION_4x := 1
ifneq (,$(filter v4.3%,$(IDF_VERSION)))
IDF_VERSION_43 := 1
else ifneq (,$(filter v4.4%,$(IDF_VERSION)))
IDF_VERSION_44 := 1
endif
else ifneq (,$(filter v5.%,$(IDF_VERSION)))
IDF_VERSION_5x := 1
endif
ifneq (,$(filter v5.0%,$(IDF_VERSION)))
IDF_VERSION_50 := 1
else ifneq (,$(filter v5.2%,$(IDF_VERSION)))
IDF_VERSION_52 := 1
endif

ifneq (,$(filter esp32s3-v4.3%,$(ESP_VARIANT)-$(IDF_VER)))
$(error esp32s3 requires ESP IDF v4.4 or later)
endif

ifneq (,$(filter esp32c2-v4%,$(ESP_VARIANT)-$(IDF_VER)))
$(error esp32c2 requires ESP IDF v5.0 or later)
endif

SDK_BUILD_BASE := $(COMPONENT_BUILD_BASE)/sdk
SDK_COMPONENT_LIBDIR := $(COMPONENT_BUILD_BASE)/lib

SDKCONFIG_H := $(SDK_BUILD_BASE)/config/sdkconfig.h

SDK_LIBDIRS := \
	esp_wifi/lib/$(ESP_VARIANT) \
	esp_coex/lib/$(ESP_VARIANT) \
	esp_phy/lib/$(ESP_VARIANT) \
	xtensa/$(ESP_VARIANT) \
	hal/$(ESP_VARIANT) \
	soc/$(ESP_VARIANT)/ld \
	esp_rom/$(ESP_VARIANT)/ld

# BLUETOOTH
ifeq ($(ESP_VARIANT),esp32)
SDK_LIBDIRS += bt/controller/lib_esp32/$(ESP_VARIANT)
ENABLE_BLUETOOTH := 1
else ifneq (,$(findstring $(ESP_VARIANT),esp32c3 esp32s3))
SDK_LIBDIRS += bt/controller/lib_esp32c3_family/$(ESP_VARIANT)
ENABLE_BLUETOOTH := 1
else
ENABLE_BLUETOOTH := 0
endif

ESP32_COMPONENT_PATH := $(COMPONENT_PATH)
SDK_DEFAULT_PATH := $(ESP32_COMPONENT_PATH)/sdk

SDK_PROJECT_PATH := $(ESP32_COMPONENT_PATH)/project/$(ESP_VARIANT)/$(BUILD_TYPE)
SDK_CONFIG_DEFAULTS := $(SDK_PROJECT_PATH)/sdkconfig.defaults

SDKCONFIG_MAKEFILE := $(SDK_PROJECT_PATH)/sdkconfig
ifeq ($(MAKE_DOCS),)
-include $(SDKCONFIG_MAKEFILE)
endif
export SDKCONFIG_MAKEFILE  # sub-makes (like bootloader) will reuse this path

ifdef IDF_VERSION_52
GLOBAL_CFLAGS += \
	-DSOC_XTAL_FREQ_MHZ=CONFIG_XTAL_FREQ \
	-DSOC_MMU_PAGE_SIZE=CONFIG_MMU_PAGE_SIZE
endif

LIBDIRS += \
	$(SDK_COMPONENT_LIBDIR) \
	$(SDK_BUILD_BASE)/esp-idf/mbedtls/mbedtls/library \
	$(SDK_BUILD_BASE)/esp-idf/$(ESP_VARIANT) \
	$(SDK_BUILD_BASE)/esp-idf/$(ESP_VARIANT)/ld \
	$(SDK_BUILD_BASE)/esp-idf/esp_system/ld \
	$(ESP32_COMPONENT_PATH)/ld \
	$(SDK_COMPONENTS_PATH)/$(ESP_VARIANT)/ld \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_LIBDIRS))

SDK_INCDIRS := \
	app_update/include \
	bootloader_support/include \
	driver/include \
	esp_pm/include \
	esp_rom/include/$(ESP_VARIANT) \
	esp_rom/include \
	esp_ringbuf/include \
	esp_timer/include \
	soc/include \
	soc/$(ESP_VARIANT)/include \
	soc/include/soc \
	heap/include \
	log/include \
	nvs_flash/include \
	esp_event/include \
	lwip/lwip/src/include \
	newlib/platform_include \
	spi_flash/include \
	wpa_supplicant/include \
	wpa_supplicant/port/include \
	esp_hw_support/include \
	esp_hw_support/include/soc \
	hal/include \
	hal/$(ESP_VARIANT)/include \
	esp_system/include \
	esp_common/include \
	esp_netif/include \
	esp_eth/include \
	esp_wifi/include \
	lwip/include/apps/sntp \
	usb/include

ifdef IDF_VERSION_4x
SDK_INCDIRS += \
	$(ESP_VARIANT)/include
endif

ifdef IDF_VERSION_43
SDK_INCDIRS += \
	esp_wifi/esp32/include \
	wpa_supplicant/include/esp_supplicant
else
SDK_INCDIRS += \
	hal/platform_port/include
endif

ifndef IDF_VERSION_52
SDK_INCDIRS += \
	driver/$(ESP_VARIANT)/include \
	lwip/port/esp32/include
endif

ifdef IDF_VERSION_4x
SDK_INCDIRS += \
	bootloader_support/include_bootloader \
	esp_adc_cal/include \
	esp_ipc/include \
	freertos/include \
	freertos/include/esp_additions \
	freertos/include/esp_additions/freertos
FREERTOS_PORTABLE := freertos/port
else
SDK_INCDIRS += \
	esp_adc/include \
	esp_app_format/include \
	esp_partition/include \
	freertos/FreeRTOS-Kernel/include \
	freertos/esp_additions/include \
	freertos/esp_additions/include/freertos \
	driver/deprecated
FREERTOS_PORTABLE := freertos/FreeRTOS-Kernel/portable
ifeq (v5.2,$(IDF_VERSION))
SDK_INCDIRS += \
	driver/gpio/include \
	driver/ledc/include \
	driver/spi/include \
	lwip/port/include \
	lwip/port/freertos/include \
	lwip/port/esp32xx/include \
	esp_bootloader_format/include \
	esp_adc/$(ESP_VARIANT)/include \
	freertos/config/include \
	freertos/config/include/freertos
endif
endif

ifeq ($(ENABLE_BLUETOOTH),1)
ifeq (esp32s3,$(ESP_VARIANT))
ESP_BT_VARIANT := esp32c3
else
ESP_BT_VARIANT := $(ESP_VARIANT)
endif
SDK_INCDIRS += \
	bt/include/$(ESP_BT_VARIANT)/include \
	bt/common/api/include/api \
	bt/common/btc/profile/esp/blufi/include  \
	bt/common/btc/profile/esp/include  \
	bt/common/osi/include	\
	bt/host/nimble/nimble/nimble/include                     \
	bt/host/nimble/nimble/nimble/host/include                \
	bt/host/nimble/nimble/porting/nimble/include             \
	bt/host/nimble/nimble/porting/npl/freertos/include       \
	bt/host/nimble/nimble/nimble/host/services/ans/include   \
	bt/host/nimble/nimble/nimble/host/services/bas/include   \
	bt/host/nimble/nimble/nimble/host/services/dis/include   \
	bt/host/nimble/nimble/nimble/host/services/gap/include   \
	bt/host/nimble/nimble/nimble/host/services/gatt/include  \
	bt/host/nimble/nimble/nimble/host/services/ias/include   \
	bt/host/nimble/nimble/nimble/host/services/ipss/include  \
	bt/host/nimble/nimble/nimble/host/services/lls/include   \
	bt/host/nimble/nimble/nimble/host/services/tps/include   \
	bt/host/nimble/nimble/nimble/host/util/include           \
	bt/host/nimble/nimble/nimble/host/store/ram/include      \
	bt/host/nimble/nimble/nimble/host/store/config/include   \
	bt/host/nimble/esp-hci/include                           \
	bt/host/nimble/port/include
ifdef IDF_VERSION_52
SDK_INCDIRS += \
	bt/host/nimble/nimble/nimble/transport/include
endif
endif

ifdef IDF_TARGET_ARCH_RISCV
SDK_INCDIRS += \
	$(FREERTOS_PORTABLE)/riscv/include \
	$(FREERTOS_PORTABLE)/riscv/include/freertos \
	freertos/config/riscv \
	freertos/config/riscv/include \
	riscv/include
else
SDK_INCDIRS += \
	xtensa/include \
	xtensa/$(ESP_VARIANT)/include \
	$(FREERTOS_PORTABLE)/xtensa/include \
	$(FREERTOS_PORTABLE)/xtensa/include/freertos
ifdef IDF_VERSION_52
SDK_INCDIRS += \
	freertos/config/xtensa \
	freertos/config/xtensa/include \
	xtensa/deprecated_include
endif
endif

	 
COMPONENT_INCDIRS += \
	$(dir $(SDKCONFIG_H)) \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_INCDIRS))


SDK_COMPONENTS := \
	app_update \
	bootloader_support \
	cxx \
	driver \
	efuse \
	esp_common \
	esp_event \
	esp_gdbstub \
	esp_hw_support \
	esp_pm \
	esp_rom \
	esp_ringbuf \
	esp_system \
	esp_timer \
	espcoredump \
	freertos \
	hal \
	heap \
	log \
	newlib \
	nvs_flash \
	pthread \
	soc \
	spi_flash

ifneq (,$(filter esp32s2 esp32s3,$(SMING_SOC)))
SDK_COMPONENTS += usb
endif

ifdef IDF_VERSION_43
SDK_COMPONENTS += $(ESP_VARIANT)
else
SDK_COMPONENTS += esp_phy
endif

ifdef IDF_VERSION_4x
SDK_COMPONENTS += \
	esp_ipc \
	esp_adc_cal
else
SDK_COMPONENTS += \
	esp_adc \
	esp_app_format \
	esp_partition
endif

ifeq (v5.2,$(IDF_VERSION))
SDK_COMPONENTS += \
	esp_mm \
	esp_coex
endif


ifneq ($(DISABLE_NETWORK),1)
SDK_COMPONENTS += \
	esp_wifi \
	esp_eth \
	lwip \
	mbedcrypto \
	esp_netif
ifneq ($(DISABLE_WIFI),1)
SDK_COMPONENTS += \
	wifi_provisioning \
	wpa_supplicant
endif
endif

ifdef IDF_TARGET_ARCH_RISCV
SDK_COMPONENTS += riscv
else
SDK_COMPONENTS += xtensa
endif

SDK_ESP_WIFI_LIBS := \
	coexist \
	core \
	espnow \
	net80211 \
	phy \
	pp \
	smartconfig

ifneq ($(ESP_VARIANT),esp32c2)
SDK_ESP_WIFI_LIBS += mesh
endif

ifeq ($(ENABLE_BLUETOOTH),1)
SDK_ESP_BLUETOOTH_LIBS := bt btdm_app
ifneq (,$(filter $(ESP_VARIANT),esp32c3 esp32s3))
SDK_ESP_BLUETOOTH_LIBS += btbb
endif
endif

ifeq ($(ESP_VARIANT),esp32)
SDK_ESP_WIFI_LIBS += rtc
endif

ifdef IDF_TARGET_ARCH_RISCV
SDK_TARGET_ARCH_LIBS := hal
else
SDK_TARGET_ARCH_LIBS := hal xt_hal
endif

EXTRA_LIBS := \
	$(SDK_COMPONENTS) \
	$(SDK_TARGET_ARCH_LIBS)

ifneq ($(DISABLE_WIFI),1)
EXTRA_LIBS += $(SDK_ESP_WIFI_LIBS)

ifeq ($(ENABLE_BLUETOOTH),1)
EXTRA_LIBS += $(SDK_ESP_BLUETOOTH_LIBS)
endif

endif

LinkerScript = -T $(ESP_VARIANT).$1.ld

LDFLAGS_esp32 := \
	$(call LinkerScript,rom.newlib-funcs) \
	$(call LinkerScript,rom.newlib-data) \
	$(call LinkerScript,rom.syscalls) \
	$(call LinkerScript,rom.eco3)

LDFLAGS_esp32s2 := \
	$(call LinkerScript,rom.newlib-funcs) \
	$(call LinkerScript,rom.newlib-data) \
	$(call LinkerScript,rom.spiflash)

LDFLAGS_esp32c3 := \
	$(call LinkerScript,rom.newlib) \
	$(call LinkerScript,rom.version) \
	$(call LinkerScript,rom.eco3)

LDFLAGS_esp32s3 := \
	$(call LinkerScript,rom.newlib) \
	$(call LinkerScript,rom.version)

LDFLAGS_esp32c2 := \
	$(call LinkerScript,rom.newlib) \
	$(call LinkerScript,rom.version) \
	$(call LinkerScript,rom.heap) \
	$(call LinkerScript,rom.mbedtls)

SDK_WRAP_SYMBOLS :=
SDK_UNDEF_SYMBOLS :=

$(foreach c,$(wildcard $(SDK_DEFAULT_PATH)/*.mk),$(eval include $c))

EXTRA_LDFLAGS := \
	$(call LinkerScript,peripherals) \
	$(call LinkerScript,rom) \
	$(call LinkerScript,rom.api) \
	$(call LinkerScript,rom.libgcc) \
	$(call LinkerScript,rom.newlib-nano) \
	$(LDFLAGS_$(ESP_VARIANT)) \
	$(call Undef,$(SDK_UNDEF_SYMBOLS)) \
	$(call Wrap,$(SDK_WRAP_SYMBOLS))

ifdef IDF_VERSION_43
EXTRA_LDFLAGS += \
	-T $(ESP_VARIANT)_out.ld \
	$(call LinkerScript,project)
else
EXTRA_LDFLAGS += \
	-T memory.ld \
	-T sections.ld
endif

FLASH_BOOT_LOADER       := $(SDK_BUILD_BASE)/bootloader/bootloader.bin
FLASH_BOOT_CHUNKS		:= $(CONFIG_BOOTLOADER_OFFSET_IN_FLASH)=$(FLASH_BOOT_LOADER)

$(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR):
	$(Q) mkdir -p $@

SDK_COMPONENT_LIBS := $(foreach c,$(SDK_COMPONENTS),$(SDK_COMPONENT_LIBDIR)/lib$c.a)

SDK_BUILD = $(ESP32_PYTHON) $(IDF_PATH)/tools/idf.py -C $(SDK_PROJECT_PATH) -B $(SDK_BUILD_BASE) -G Ninja

# For misc.mk / copylibs
export SDK_BUILD_BASE
export SDK_COMPONENT_LIBDIR
export SDK_COMPONENTS

ifndef MAKE_CLEAN
.PHONY: checkdirs
checkdirs: | checksdk	

.PHONY: checksdk
checksdk: $(SDK_PROJECT_PATH) $(SDKCONFIG_H) $(SDKCONFIG_MAKEFILE)
	$(Q) $(NINJA) -C $(SDK_BUILD_BASE) bootloader app
	$(Q) $(MAKE) --no-print-directory -C $(ESP32_COMPONENT_PATH) -f misc.mk copylibs

.PHONY: reconfigure-sdk
reconfigure-sdk:
	$(Q) $(SDK_BUILD) reconfigure

$(COMPONENT_NAME)-build: reconfigure-sdk

$(SDKCONFIG_H) $(SDKCONFIG_MAKEFILE) $(SDK_COMPONENT_LIBS): $(SDK_PROJECT_PATH) $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR)
	$(Q) $(SDK_BUILD) reconfigure

$(SDK_PROJECT_PATH):
	$(Q) mkdir -p $@
	$(Q) cp -r $(SDK_DEFAULT_PATH)/project/* $@

$(SDK_COMPONENT_LIBS): checksdk
endif

SDK_CONFIG_FILES := \
	common \
	$(BUILD_TYPE) \
	$(ESP_VARIANT).common \
	$(ESP_VARIANT).$(BUILD_TYPE)

ifdef SDK_CUSTOM_CONFIG
SDK_CUSTOM_CONFIG_PATH := $(call AbsoluteSourcePath,$(PROJECT_DIR),$(SDK_CUSTOM_CONFIG))
endif

SDK_CONFIG_FILES := \
	$(addprefix $(SDK_DEFAULT_PATH)/config/,$(SDK_CONFIG_FILES)) \
	$(SDK_CUSTOM_CONFIG_PATH)

$(SDK_CONFIG_DEFAULTS): $(SDK_CUSTOM_CONFIG_PATH)
	@echo Creating $@
	$(Q) rm -f $@
	$(Q) $(foreach f,$(SDK_CONFIG_FILES),\
		$(if $(wildcard $f),cat $f >> $@;) \
	)
	$(Q) printf "\n# Auto-generated settings\n" >> $@
	$(Q) echo "CONFIG_ESPTOOLPY_FLASHMODE_$(SPI_MODE)=y" >> $@
	$(Q) echo "CONFIG_ESPTOOLPY_FLASHFREQ_$(SPI_SPEED)M=y" >> $@
	$(Q) echo "CONFIG_ESPTOOLPY_FLASHSIZE_$(SPI_SIZE)B=y" >> $@
ifeq ($(ENABLE_GDB), 1)
	$(Q) echo "CONFIG_ESP_SYSTEM_PANIC_GDBSTUB=$(if $(ENABLE_GDB),y,n)" >> $@
endif

##@Configuration

PHONY: sdk-menuconfig
sdk-menuconfig: $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) ##Configure SDK options
	$(Q) $(SDK_BUILD) menuconfig
	@echo Now run 'make'

##@Cleaning

.PHONY: sdk-config-clean
sdk-config-clean: esp32-clean ##Wipe SDK configuration and revert to defaults
	$(Q) rm -rf $(SDK_PROJECT_PATH)

##@Help

.PHONY: sdk-help
sdk-help: ##Get SDK build options
	$(Q) $(SDK_BUILD) --help

##@Tools

.PHONY: sdk
sdk: ##Pass options to IDF builder, e.g. `make sdk -- --help` or `make sdk menuconfig` 
	$(Q) $(SDK_BUILD) $(filter-out sdk,$(MAKECMDGOALS))


.PHONY: check-incdirs
check-incdirs: ##Check IDF include paths and report any not found in this SDK version
	$(Q) $(foreach d,$(SDK_INCDIRS),$(if $(wildcard $(SDK_COMPONENTS_PATH)/$d),,$(info $d)))
