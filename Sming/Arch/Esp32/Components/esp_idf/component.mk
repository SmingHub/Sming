COMPONENT_LIBNAME   :=
IDF_COMPONENTS_PATH := $(IDF_PATH)/components

SDK_BUILD_BASE := $(SMING_HOME)/$(BUILD_BASE)/sdk

LIBDIRS += $(SDK_BUILD_BASE)/lib $(SDK_BUILD_BASE)/esp32

#include $(IDF_COMPONENTS_PATH)/esp_rom/component.mk
#include $(IDF_COMPONENTS_PATH)/heap/component.mk

LIBDIRS += $(IDF_COMPONENTS_PATH)/esp_wifi/lib_esp32 \
           $(IDF_COMPONENTS_PATH)/xtensa/esp32/ \
           $(IDF_COMPONENTS_PATH)/esp32/ld \
           $(IDF_COMPONENTS_PATH)/esp_rom/esp32/ld

COMPONENT_INCDIRS := \
	$(SDK_BUILD_BASE)/include \
	$(IDF_COMPONENTS_PATH)/bootloader_support/include \
	$(IDF_COMPONENTS_PATH)/bootloader_support/include_bootloader \
	$(IDF_COMPONENTS_PATH)/console \
	$(IDF_COMPONENTS_PATH)/driver/include \
	$(IDF_COMPONENTS_PATH)/driver/include/driver \
	$(IDF_COMPONENTS_PATH)/efuse/include \
	$(IDF_COMPONENTS_PATH)/efuse/esp32/include \
	$(IDF_COMPONENTS_PATH)/esp32/include \
	$(IDF_COMPONENTS_PATH)/espcoredump/include \
	$(IDF_COMPONENTS_PATH)/soc/include \
	$(IDF_COMPONENTS_PATH)/soc/esp32/include \
	$(IDF_COMPONENTS_PATH)/heap/include \
	$(IDF_COMPONENTS_PATH)/log/include \
	$(IDF_COMPONENTS_PATH)/nvs_flash/include \
	$(IDF_COMPONENTS_PATH)/freertos/include \
	$(IDF_COMPONENTS_PATH)/esp_ringbuf/include \
	$(IDF_COMPONENTS_PATH)/esp_event/include \
	$(IDF_COMPONENTS_PATH)/tcpip_adapter/include \
	$(IDF_COMPONENTS_PATH)/lwip/lwip/src/include \
	$(IDF_COMPONENTS_PATH)/lwip/port/esp32/include \
	$(IDF_COMPONENTS_PATH)/lwip/include/apps \
	$(IDF_COMPONENTS_PATH)/lwip/include/apps/sntp \
	$(IDF_COMPONENTS_PATH)/mbedtls/mbedtls/include \
	$(IDF_COMPONENTS_PATH)/mbedtls/port/include \
	$(IDF_COMPONENTS_PATH)/mdns/include \
	$(IDF_COMPONENTS_PATH)/mdns/private_include \
	$(IDF_COMPONENTS_PATH)/spi_flash/include \
	$(IDF_COMPONENTS_PATH)/ulp/include \
	$(IDF_COMPONENTS_PATH)/vfs/include \
	$(IDF_COMPONENTS_PATH)/xtensa-debug-module/include \
	$(IDF_COMPONENTS_PATH)/wpa_supplicant/include \
	$(IDF_COMPONENTS_PATH)/wpa_supplicant/port/include \
	$(IDF_COMPONENTS_PATH)/app_trace/include \
	$(IDF_COMPONENTS_PATH)/app_update/include \
	$(IDF_COMPONENTS_PATH)/pthread/include \
	$(IDF_COMPONENTS_PATH)/smartconfig_ack/include \
	$(IDF_COMPONENTS_PATH)/sdmmc/include \
	\
	$(IDF_COMPONENTS_PATH)/esp_common/include \
	$(IDF_COMPONENTS_PATH)/esp_adc_cal/include \
	$(IDF_COMPONENTS_PATH)/esp_eth/include \
	$(IDF_COMPONENTS_PATH)/esp_event/private_include \
	$(IDF_COMPONENTS_PATH)/esp_rom/include \
	$(IDF_COMPONENTS_PATH)/esp_wifi/include \
	$(IDF_COMPONENTS_PATH)/esp_wifi/esp32/include \
	$(IDF_COMPONENTS_PATH)/lwip/include/apps/sntp \
	$(IDF_COMPONENTS_PATH)/spi_flash/private_include \
	$(IDF_COMPONENTS_PATH)/wpa_supplicant/include/esp_supplicant \
	$(IDF_COMPONENTS_PATH)/xtensa/include \
	$(IDF_COMPONENTS_PATH)/xtensa/esp32/include
	
ifeq ($(CONFIG_BT_NIMBLE_ENABLED),y)
	COMPONENT_INCDIRS += \
    	$(IDF_COMPONENTS_PATH)/bt/include \
    	$(IDF_COMPONENTS_PATH)/bt/common/osi/include \
    	$(IDF_COMPONENTS_PATH)/bt/common/btc/include \
    	$(IDF_COMPONENTS_PATH)/bt/common/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/porting/nimble/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/port/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/ans/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/bas/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/gap/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/gatt/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/ias/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/lls/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/services/tps/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/util/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/store/ram/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/nimble/host/store/config/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/porting/npl/freertos/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/nimble/ext/tinycrypt/include \
    	$(IDF_COMPONENTS_PATH)/bt/host/nimble/esp-hci/include
endif

COMPONENT_INCDIRS += \
	 $(IDF_COMPONENTS_PATH)/newlib/platform_include
	 
EXTRA_LIBS := app_trace \
            app_update \
            asio \
            bootloader_support \
            bt \
            coap \
            console \
            cxx \
            driver \
            efuse \
            esp-tls \
            esp32 \
            esp_adc_cal \
            esp_common \
            esp_eth \
            esp_event \
            esp_gdbstub \
            esp_http_client \
            esp_http_server \
            esp_https_ota \
            esp_local_ctrl \
            esp_ringbuf \
            esp_rom \
            esp_websocket_client \
            esp_wifi \
            core \
            rtc \
            net80211 \
            pp \
            smartconfig \
            coexist \
            espnow \
            phy \
            mesh \
            espcoredump \
            expat \
            fatfs \
            freemodbus \
            freertos \
            heap \
            idf_test \
            jsmn \
            json \
            libsodium \
            log \
            lwip \
            mbedtls \
            mdns \
            mqtt \
            newlib \
            c \
            m  \
            nghttp \
            nvs_flash \
            openssl \
            protobuf-c \
            protocomm \
            pthread \
            sdmmc \
            soc \
            spi_flash \
            spiffs \
            tcp_transport \
            tcpip_adapter \
            ulp \
            unity \
            vfs \
            wear_levelling \
            wifi_provisioning \
            wpa_supplicant \
            xtensa \
            hal \
            gcc \
            stdc++ \
            gcov
            
EXTRA_LDFLAGS := -u esp_app_desc \
            -u __cxa_guard_dummy -u __cxx_fatal_exception \
            -T esp32_out.ld -u ld_include_panic_highint_hdl \
            -T esp32.project.ld \
            -T esp32.peripherals.ld  \
            -T esp32.rom.ld \
            -T esp32.rom.libgcc.ld \
            -T esp32.rom.syscalls.ld \
            -T esp32.rom.newlib-data.ld \
            -T esp32.rom.newlib-funcs.ld  \
            -u newlib_include_locks_impl -u newlib_include_heap_impl -u newlib_include_syscalls_impl \
            -u pthread_include_pthread_impl -u pthread_include_pthread_cond_impl -u pthread_include_pthread_local_storage_impl \
            -Wl,--undefined=uxTopUsedPriority        
            
FLASH_BOOT_LOADER       := $(SDK_BUILD_BASE)/bootloader/bootloader.bin
FLASH_BOOT_CHUNKS		:= 0x1000=$(FLASH_BOOT_LOADER)

CACHE_VARS				+= COM_PORT_ESPTOOL COM_SPEED_ESPTOOL
COM_PORT_ESPTOOL		?= $(COM_PORT)
COM_SPEED_ESPTOOL		?= $(COM_SPEED)
ESPTOOL_CMDLINE			:= $(PYTHON) $(IDF_COMPONENTS_PATH)/esptool_py/esptool/esptool.py --chip esp32 --port $(COM_PORT_ESPTOOL) --baud $(COM_SPEED_ESPTOOL) --before default_reset --after hard_reset

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(info WriteFlash $1)
	$(ESPTOOL_CMDLINE) write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect $(flashimageoptions) $(subst =, ,$1)
endef

SDK_DEFAULT_PATH := $(COMPONENT_PATH)/sdk
     
##@Partitions

SDK_PARTITION_PATH := $(SDK_DEFAULT_PATH)/partitions

##@SDK

SDK_PROJECT_PATH := $(COMPONENT_PATH)/project
SDK_CONFIG_DEFAULTS := $(SDK_PROJECT_PATH)/sdkconfig.defaults

SDKCONFIG_MAKEFILE ?= $(SDK_PROJECT_PATH)/sdkconfig
-include $(SDKCONFIG_MAKEFILE)
export SDKCONFIG_MAKEFILE  # sub-makes (like bootloader) will reuse this path

$(SDK_BUILD_BASE):
	$(Q) mkdir -p $@
	
checksdk:
	$(Q) if [ ! -f $(SDK_BUILD_BASE)/include/sdkconfig.h ]; then \
		$(MAKE) sdk-default; \
	fi 
	
.PHONY: sdk-default 
sdk-default: $(SDK_BUILD_BASE)##Installs default ESP-IDF SDK
	$(Q) echo "Installing pre-compiled SDK."
	$(Q) rm -rf $<; 
	$(Q) cp -r $(SDK_DEFAULT_PATH) $(SDK_BUILD_BASE)

$(SDK_CONFIG_DEFAULTS):
	$(Q) cp $@.$(BUILD_TYPE) $@

.PHONY: sdk-menuconfig
sdk-menuconfig: $(SDK_BUILD_BASE) $(SDK_CONFIG_DEFAULTS)##Reconfigure ESP-IDF SDK. To (re-)build it run `make sdk-build`
	$(MAKE) -C $(SDK_PROJECT_PATH) menuconfig BUILD_DIR_BASE=$(SDK_BUILD_BASE)
	
.PHONY: sdk-menuconfig-clean
sdk-menuconfig-clean: 
	$(Q) rm -f $(SDKCONFIG_MAKEFILE) $(SDK_CONFIG_DEFAULTS) 

.PHONY: sdk-build
sdk-build: $(SDK_PROJECT_PATH)/sdkconfig $(SDK_BUILD_BASE) ##Build ESP-IDF SDK
	$(Q) rm -f $(FLASH_BOOT_LOADER)
	-$(MAKE) -C $(SDK_PROJECT_PATH) BUILD_DIR_BASE=$(SDK_BUILD_BASE)
	$(Q) mkdir -p $(SDK_BUILD_BASE)/lib
	$(Q) rm -rf $(SDK_BUILD_BASE)/lib/*
	$(Q) for i in $$(find $(SDK_BUILD_BASE) -name '*.a'); do \
		cp $$i $(SDK_BUILD_BASE)/lib/.; \
	done
	
.PHONY: sdk-clean
sdk-clean: ##Removes the SDK 
	$(Q) rm -rf $(SDK_BUILD_BASE)
            
checkdirs: | checksdk	
			            
