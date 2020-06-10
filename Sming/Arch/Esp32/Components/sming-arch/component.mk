
IDF_TARGET ?= esp32

COMPONENT_SRCDIRS := \
	$(ARCH_CORE) $(call ListAllSubDirs,$(ARCH_CORE)) \
	$(ARCH_SYS) \
	$(ARCH_BASE)/Platform

COMPONENT_INCDIRS := \
	$(ARCH_BASE) \
	$(ARCH_CORE) \
	$(ARCH_SYS)/include \
	$(ARCH_COMPONENTS)

COMPONENT_DEPENDS := \
	esp_hal \
	libc \
	spi_flash \
	esp32 \
	esp_idf \
	driver \
	heap \
	fatfs \
	esp_spiffs

#
DISABLE_WIFI ?= 0
ifeq ($(DISABLE_WIFI),1)
GLOBAL_CFLAGS += -DDISABLE_WIFI=1
endif

# => Platform WiFi
COMPONENT_VARS := \
	ENABLE_WPS \
	ENABLE_SMART_CONFIG \
	DISABLE_WIFI


# ELF and BIN files
TARGET_OUT := $(BUILD_BASE)/$(APP_NAME).out
TARGET_BIN := $(FW_BASE)/$(APP_NAME).bin
TARGET_OUT_0 := $(TARGET_OUT) 
