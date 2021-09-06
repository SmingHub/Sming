SDK_COMPONENTS_PATH := $(IDF_PATH)/components

# See build.mk for default ESP_VARIANT - determines toolchain

COMPONENT_DEPENDS := libc

COMPONENT_SRCDIRS := src
COMPONENT_INCDIRS := src/include include

# Set to build all components, otherwise just the core ones
CACHE_VARS += SDK_FULL_BUILD
SDK_FULL_BUILD ?= 0

# Applications can provide file with custom SDK configuration settings
CACHE_VARS += SDK_CUSTOM_CONFIG

COMPONENT_RELINK_VARS += DISABLE_NETWORK DISABLE_WIFI

SDK_BUILD_BASE := $(COMPONENT_BUILD_BASE)/$(ESP_VARIANT)/sdk
SDK_COMPONENT_LIBDIR := $(COMPONENT_BUILD_BASE)/$(ESP_VARIANT)/lib

SDKCONFIG_H := $(SDK_BUILD_BASE)/config/sdkconfig.h

SDK_LIBDIRS := \
	esp_wifi/lib/$(ESP_VARIANT) \
	xtensa/$(ESP_VARIANT)/ \
	hal/$(ESP_VARIANT)/ \
	$(ESP_VARIANT)/ld \
	esp_rom/$(ESP_VARIANT)/ld

LIBDIRS += \
	$(SDK_COMPONENT_LIBDIR) \
	$(SDK_BUILD_BASE)/esp-idf/mbedtls/mbedtls/library \
	$(SDK_BUILD_BASE)/esp-idf/$(ESP_VARIANT) \
	$(SDK_BUILD_BASE)/esp-idf/$(ESP_VARIANT)/ld \
	$(COMPONENT_PATH)/ld \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_LIBDIRS))

SDK_INCDIRS := \
	bootloader_support/include \
	bootloader_support/include_bootloader \
	driver/$(ESP_VARIANT)/include \
	driver/include \
	efuse/include \
	efuse/$(ESP_VARIANT)/include \
	esp_rom/include/$(ESP_VARIANT) \
	esp_rom/include \
	$(ESP_VARIANT)/include \
	espcoredump/include \
	esp_timer/include \
	soc/include \
	soc/$(ESP_VARIANT)/include \
	heap/include \
	log/include \
	nvs_flash/include \
	freertos/include \
	esp_ringbuf/include \
	esp_event/include \
	tcpip_adapter/include \
	lwip/lwip/src/include \
	lwip/port/esp32/include \
	lwip/include/apps \
	lwip/include/apps/sntp \
	mbedtls/mbedtls/include \
	mbedtls/port/include \
	mdns/include \
	mdns/private_include \
	spi_flash/include \
	ulp/include \
	xtensa-debug-module/include \
	wpa_supplicant/include \
	wpa_supplicant/port/include \
	app_trace/include \
	app_update/include \
	smartconfig_ack/include \
	esp_hw_support/include \
	hal/include \
	hal/$(ESP_VARIANT)/include \
	esp_system/include

ifeq ($(SDK_FULL_BUILD),1)
SDK_INCDIRS += \
	console \
	pthread/include \
	sdmmc/include
endif

SDK_INCDIRS += \
	esp_common/include \
	esp_adc_cal/include \
	esp_netif/include \
	esp_eth/include \
	esp_event/private_include \
	esp_wifi/include \
	esp_wifi/esp32/include \
	lwip/include/apps/sntp \
	spi_flash/private_include \
	wpa_supplicant/include/esp_supplicant

ifdef IDF_TARGET_ARCH_RISCV
SDK_INCDIRS += \
	freertos/port/riscv/include \
	riscv/include
else
SDK_INCDIRS += \
	xtensa/include \
	xtensa/$(ESP_VARIANT)/include \
	freertos/port/xtensa/include
endif

ifeq ($(CONFIG_BT_NIMBLE_ENABLED),y)
SDK_INCDIRS += \
	bt/include \
	bt/common/osi/include \
	bt/common/btc/include \
	bt/common/include \
	bt/host/nimble/nimble/porting/nimble/include \
	bt/host/nimble/port/include \
	bt/host/nimble/nimble/nimble/include \
	bt/host/nimble/nimble/nimble/host/include \
	bt/host/nimble/nimble/nimble/host/services/ans/include \
	bt/host/nimble/nimble/nimble/host/services/bas/include \
	bt/host/nimble/nimble/nimble/host/services/gap/include \
	bt/host/nimble/nimble/nimble/host/services/gatt/include \
	bt/host/nimble/nimble/nimble/host/services/ias/include \
	bt/host/nimble/nimble/nimble/host/services/lls/include \
	bt/host/nimble/nimble/nimble/host/services/tps/include \
	bt/host/nimble/nimble/nimble/host/util/include \
	bt/host/nimble/nimble/nimble/host/store/ram/include \
	bt/host/nimble/nimble/nimble/host/store/config/include \
	bt/host/nimble/nimble/porting/npl/freertos/include \
	bt/host/nimble/nimble/ext/tinycrypt/include \
	bt/host/nimble/esp-hci/include
endif

SDK_INCDIRS += \
	 newlib/platform_include
	 
COMPONENT_INCDIRS += \
	$(dir $(SDKCONFIG_H)) \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_INCDIRS))


SDK_COMPONENTS := \
	app_update \
	bootloader_support \
	cxx \
	driver \
	efuse \
	$(ESP_VARIANT) \
	esp_common \
	esp_event \
	esp_gdbstub \
	esp_hw_support \
	esp_ipc \
	esp_pm \
	esp_rom \
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

ifneq ($(DISABLE_NETWORK),1)
SDK_COMPONENTS += \
	esp_wifi \
	esp_eth \
	lwip \
	mbedtls \
	mbedcrypto \
	esp_netif \
	openssl
ifneq ($(DISABLE_WIFI),1)
SDK_COMPONENTS += \
	wifi_provisioning \
	wpa_supplicant
endif
endif

ifneq ($(ESP_VARIANT),esp32s3)
SDK_COMPONENTS += esp_adc_cal
endif

ifdef IDF_TARGET_ARCH_RISCV
SDK_COMPONENTS += riscv
else
SDK_COMPONENTS += xtensa
endif

ifeq ($(SDK_FULL_BUILD),1)
SDK_COMPONENTS += \
	app_trace \
	asio \
	coap \
	console \
	esp_http_client \
	esp_http_server \
	esp_https_ota \
	esp_local_ctrl \
	esp_ringbuf \
	esp_websocket_client \
	expat \
	fatfs \
	freemodbus \
	jsmn \
	json \
	libsodium \
	mdns \
	mqtt \
	nghttp \
	protobuf-c \
	protocomm \
	sdmmc \
	ulp \
	unity \
	vfs \
	wear_levelling
endif

SDK_ESP_WIFI_LIBS := \
	coexist \
	core \
	espnow \
	mesh \
	net80211 \
	phy \
	pp \
	smartconfig

ifeq ($(ESP_VARIANT),esp32)
SDK_ESP_WIFI_LIBS += rtc
endif

SDK_NEWLIB_LIBS := \
	c \
	m  \
	stdc++

ifdef IDF_TARGET_ARCH_RISCV
SDK_TARGET_ARCH_LIBS := hal
else
SDK_TARGET_ARCH_LIBS := hal xt_hal
endif

EXTRA_LIBS := \
	gcc \
	$(SDK_COMPONENTS) \
	$(SDK_NEWLIB_LIBS) \
	$(SDK_TARGET_ARCH_LIBS)

ifneq ($(DISABLE_WIFI),1)
EXTRA_LIBS += $(SDK_ESP_WIFI_LIBS)
endif

LinkerScript = -T $(ESP_VARIANT).$1.ld

LDFLAGS_esp32 := \
	$(call LinkerScript,rom.newlib-funcs) \
	$(call LinkerScript,rom.newlib-data) \
	$(call LinkerScript,rom.syscalls) \
	$(call LinkerScript,rom.newlib-time) \
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
	$(call LinkerScript,rom.newlib-funcs) \
	$(call LinkerScript,rom.newlib-data) \
	$(call LinkerScript,rom.spiflash)

EXTRA_LDFLAGS := \
	-u esp_app_desc \
	-u __cxa_guard_dummy -u __cxx_fatal_exception \
	-T $(ESP_VARIANT)_out.ld \
	-u ld_include_panic_highint_hdl \
	$(call LinkerScript,project) \
	$(call LinkerScript,peripherals) \
	$(call LinkerScript,rom) \
	$(call LinkerScript,rom.api) \
	$(call LinkerScript,rom.libgcc) \
	-u newlib_include_locks_impl \
	-u newlib_include_heap_impl \
	-u newlib_include_syscalls_impl \
	-u pthread_include_pthread_impl \
	-u pthread_include_pthread_cond_impl \
	-u pthread_include_pthread_local_storage_impl \
	-Wl,--undefined=uxTopUsedPriority \
	$(LDFLAGS_$(ESP_VARIANT))

FLASH_BOOT_LOADER       := $(SDK_BUILD_BASE)/bootloader/bootloader.bin
FLASH_BOOT_CHUNKS		:= 0x1000=$(FLASH_BOOT_LOADER)

SDK_DEFAULT_PATH := $(COMPONENT_PATH)/sdk

##@SDK

SDK_PROJECT_PATH := $(COMPONENT_PATH)/project/$(ESP_VARIANT)/$(BUILD_TYPE)
SDK_CONFIG_DEFAULTS := $(SDK_PROJECT_PATH)/sdkconfig.defaults

SDKCONFIG_MAKEFILE := $(SDK_PROJECT_PATH)/sdkconfig
ifeq ($(MAKE_DOCS),)
-include $(SDKCONFIG_MAKEFILE)
endif
export SDKCONFIG_MAKEFILE  # sub-makes (like bootloader) will reuse this path

$(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR):
	$(Q) mkdir -p $@

SDK_COMPONENT_LIBS := $(foreach c,$(SDK_COMPONENTS),$(SDK_COMPONENT_LIBDIR)/lib$c.a)

SDK_BUILD_COMPLETE := $(SDK_BUILD_BASE)/.complete

CUSTOM_TARGETS += checksdk

.PHONY: checksdk
checksdk: $(SDK_PROJECT_PATH) $(SDK_BUILD_COMPLETE)

SDK_BUILD = $(ESP32_PYTHON) $(IDF_PATH)/tools/idf.py -C $(SDK_PROJECT_PATH) -B $(SDK_BUILD_BASE) -G Ninja

# For misc.mk / copylibs
export SDK_BUILD_BASE
export SDK_COMPONENT_LIBDIR
export SDK_COMPONENTS

$(SDK_BUILD_COMPLETE): $(SDKCONFIG_H) $(SDKCONFIG_MAKEFILE)
	$(Q) $(SDK_BUILD) reconfigure
	$(Q) $(NINJA) -C $(SDK_BUILD_BASE) bootloader app
	$(Q) $(MAKE) --no-print-directory -C $(SDK_DEFAULT_PATH) -f misc.mk copylibs
	touch $(SDK_BUILD_COMPLETE)

$(SDKCONFIG_H) $(SDKCONFIG_MAKEFILE) $(SDK_COMPONENT_LIBS): $(SDK_PROJECT_PATH) $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR)

$(SDK_PROJECT_PATH):
	$(Q) mkdir -p $@
	$(Q) cp -r $(SDK_DEFAULT_PATH)/project/* $@

$(SDK_COMPONENT_LIBS): $(SDK_BUILD_COMPLETE)

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

PHONY: sdk-menuconfig
sdk-menuconfig: $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) ##Configure SDK options
	$(Q) $(SDK_BUILD) menuconfig
	$(Q) rm -f $(SDK_BUILD_COMPLETE)
	@echo Now run 'make esp32-build'

.PHONY: sdk-config-clean
sdk-config-clean: esp32-clean ##Wipe SDK configuration and revert to defaults
	$(Q) rm -rf $(SDK_PROJECT_PATH)

.PHONY: sdk-help
sdk-help: ##Get SDK build options
	$(Q) $(SDK_BUILD) --help

.PHONY: sdk
sdk: ##Pass options to IDF builder, e.g. `make sdk -- --help` or `make sdk menuconfig` 
	$(Q) $(SDK_BUILD) $(filter-out sdk,$(MAKECMDGOALS))

.PHONY: checkdirs   
checkdirs: | checksdk	
			            
