SDK_COMPONENTS_PATH := $(IDF_PATH)/components

# See build.mk for default ESP_VARIANT - determines toolchain
ifeq (esp32s2,$(ESP_VARIANT))
SDK_BUILD_VARIANT := esp32s2beta
else
SDK_BUILD_VARIANT := $(ESP_VARIANT)
endif

COMPONENT_DEPENDS := libc

COMPONENT_SRCDIRS := src
COMPONENT_INCDIRS := src/include include

# Set to build all components, otherwise just the core ones
CACHE_VARS += SDK_FULL_BUILD
SDK_FULL_BUILD ?= 0

SDK_BUILD_BASE := $(COMPONENT_BUILD_DIR)/sdk
SDK_COMPONENT_LIBDIR := $(COMPONENT_BUILD_DIR)/lib

SDKCONFIG_H := $(SDK_BUILD_BASE)/config/sdkconfig.h

SDK_LIBDIRS := \
	esp_wifi/lib/$(SDK_BUILD_VARIANT) \
	xtensa/$(SDK_BUILD_VARIANT)/ \
	esp32/ld \
	esp_rom/esp32/ld

LIBDIRS += \
	$(SDK_COMPONENT_LIBDIR) \
	$(SDK_BUILD_BASE)/esp-idf/$(SDK_BUILD_VARIANT) \
	$(SDK_BUILD_BASE)/esp-idf/$(SDK_BUILD_VARIANT)/ld \
	$(COMPONENT_PATH)/ld \
	$(addprefix $(SDK_COMPONENTS_PATH)/,$(SDK_LIBDIRS))

SDK_INCDIRS := \
	bootloader_support/include \
	bootloader_support/include_bootloader \
	driver/include \
	driver/include/driver \
	efuse/include \
	efuse/esp32/include \
	esp32/include \
	espcoredump/include \
	soc/include \
	soc/esp32/include \
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
	vfs/include \
	xtensa-debug-module/include \
	wpa_supplicant/include \
	wpa_supplicant/port/include \
	app_trace/include \
	app_update/include \
	smartconfig_ack/include

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
	esp_rom/include \
	esp_wifi/include \
	esp_wifi/esp32/include \
	lwip/include/apps/sntp \
	spi_flash/private_include \
	wpa_supplicant/include/esp_supplicant \
	xtensa/include \
	xtensa/esp32/include
	
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
	esp-tls \
	esp32 \
	esp_adc_cal \
	esp_common \
	esp_event \
	esp_gdbstub \
	esp_ringbuf \
	esp_wifi \
	espcoredump \
	freertos \
	heap \
	log \
	lwip \
	mbedtls \
	mbedcrypto \
	esp_netif \
	newlib \
	nvs_flash \
	openssl \
	protobuf-c \
	protocomm \
	pthread \
	soc \
	spi_flash \
	tcp_transport \
	tcpip_adapter \
	vfs \
	wifi_provisioning \
	wpa_supplicant \
	xtensa

ifeq ($(SDK_FULL_BUILD),1)
SDK_COMPONENTS += \
	app_trace \
	asio \
	bt \
	coap \
	console \
	esp_eth \
	esp_http_client \
	esp_http_server \
	esp_https_ota \
	esp_local_ctrl \
	esp_websocket_client \
	expat \
	fatfs \
	freemodbus \
	idf_test \
	jsmn \
	json \
	libsodium \
	mdns \
	mqtt \
	nghttp \
	sdmmc \
	ulp \
	unity \
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
	rtc \
	smartconfig

SDK_NEWLIB_LIBS := \
	c \
	m  \
	stdc++

SDK_XTENSA_LIBS := \
	hal

EXTRA_LIBS := \
	gcc \
	$(SDK_COMPONENTS) \
	$(SDK_ESP_WIFI_LIBS) \
	$(SDK_NEWLIB_LIBS) \
	$(SDK_XTENSA_LIBS)

EXTRA_LDFLAGS := \
	-u esp_app_desc \
	-u __cxa_guard_dummy -u __cxx_fatal_exception \
	-T esp32_out.ld \
	-u ld_include_panic_highint_hdl \
	-T esp32.project.ld \
	-T esp32.peripherals.ld  \
	-T esp32.rom.ld \
	-T esp32.rom.libgcc.ld \
	-T esp32.rom.syscalls.ld \
	-T esp32.rom.newlib-data.ld \
	-T esp32.rom.newlib-funcs.ld  \
	-u newlib_include_locks_impl \
	-u newlib_include_heap_impl \
	-u newlib_include_syscalls_impl \
	-u pthread_include_pthread_impl \
	-u pthread_include_pthread_cond_impl \
	-u pthread_include_pthread_local_storage_impl \
	-Wl,--undefined=uxTopUsedPriority        
            
FLASH_BOOT_LOADER       := $(SDK_BUILD_BASE)/bootloader/bootloader.bin
FLASH_BOOT_CHUNKS		:= 0x1000=$(FLASH_BOOT_LOADER)

CACHE_VARS				+= COM_PORT_ESPTOOL COM_SPEED_ESPTOOL
COM_PORT_ESPTOOL		?= $(COM_PORT)
COM_SPEED_ESPTOOL		?= $(COM_SPEED)
ESPTOOL_CMDLINE := \
	$(PYTHON) $(SDK_COMPONENTS_PATH)/esptool_py/esptool/esptool.py --chip esp32 \
	--port $(COM_PORT_ESPTOOL) --baud $(COM_SPEED_ESPTOOL) \
	--before default_reset --after hard_reset

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(info WriteFlash $1)
	$(ESPTOOL_CMDLINE) write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect $(flashimageoptions) $(subst =, ,$1)
endef

SDK_DEFAULT_PATH := $(COMPONENT_PATH)/sdk


##@Debugging

CACHE_VARS += GDB_CMDLINE
GDB_CMDLINE = trap '' INT; $(GDB) -x $(ARCH_TOOLS)/gdbinit $(TARGET_OUT)
     
##@Partitions

SDK_PARTITION_PATH := $(SDK_DEFAULT_PATH)/partitions

##@SDK

SDK_PROJECT_PATH := $(COMPONENT_PATH)/project
SDK_CONFIG_DEFAULTS := $(SDK_PROJECT_PATH)/sdkconfig.defaults

SDKCONFIG_MAKEFILE ?= $(SDK_PROJECT_PATH)/sdkconfig
-include $(SDKCONFIG_MAKEFILE)
export SDKCONFIG_MAKEFILE  # sub-makes (like bootloader) will reuse this path

$(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR):
	$(Q) mkdir -p $@

SDK_COMPONENT_LIBS := $(foreach c,$(SDK_COMPONENTS),$(SDK_COMPONENT_LIBDIR)/lib$c.a)

SDK_BUILD_COMPLETE := $(SDK_BUILD_BASE)/.complete

CUSTOM_TARGETS += checksdk

.PHONY: checksdk
checksdk: $(SDK_BUILD_COMPLETE)

SDK_BUILD = $(ESP32_PYTHON) $(IDF_PATH)/tools/idf.py -C $(SDK_PROJECT_PATH) -B $(SDK_BUILD_BASE) -G Ninja

# For misc.mk / copylibs
export SDK_BUILD_BASE
export SDK_COMPONENT_LIBDIR
export SDK_COMPONENTS

$(SDK_BUILD_COMPLETE): $(SDKCONFIG_H) $(SDK_COMPONENT_LIBS)
	touch $(SDK_BUILD_COMPLETE)

$(SDKCONFIG_H) $(SDK_COMPONENT_LIBS): $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) $(SDK_COMPONENT_LIBDIR)
	$(Q) $(SDK_BUILD) bootloader app
	$(Q) $(MAKE) --no-print-directory -C $(SDK_DEFAULT_PATH) -f misc.mk copylibs

$(SDK_CONFIG_DEFAULTS):
	$(Q) cp $@.$(BUILD_TYPE) $@

PHONY: sdk-menuconfig
sdk-menuconfig: $(SDK_CONFIG_DEFAULTS) | $(SDK_BUILD_BASE) ##Configure SDK options
	$(SDK_BUILD) menuconfig
	@echo Now run 'make esp32-build'

.PHONY: sdk-defconfig
sdk-defconfig: $(SDK_BUILD_BASE)/include/sdkconfig.h ##Create default SDK config files

.PHONY: sdk-menuconfig-clean
sdk-menuconfig-clean:
	$(Q) rm -f $(SDKCONFIG_MAKEFILE) $(SDK_CONFIG_DEFAULTS) 

.PHONY: sdk-help
sdk-help: ##Get SDK build options
	$(Q) $(SDK_BUILD) --help

.PHONY: sdk
sdk: ##Pass options to IDF builder, e.g. `make sdk -- --help` or `make sdk menuconfig` 
	$(Q) $(SDK_BUILD) $(filter-out sdk,$(MAKECMDGOALS))

.PHONY: checkdirs   
checkdirs: | checksdk	
			            
