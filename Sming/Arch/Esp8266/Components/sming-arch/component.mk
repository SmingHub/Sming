COMPONENT_SRCDIRS := \
	$(ARCH_CORE) $(call ListAllSubDirs,$(ARCH_CORE)) \
	$(ARCH_SYS) \
	$(ARCH_BASE)/Platform \
	$(ARCH_BASE)/Services/Profiling

COMPONENT_INCDIRS := \
	$(ARCH_BASE) \
	$(ARCH_CORE) \
	$(ARCH_SYS)/include \
	$(ARCH_COMPONENTS)

COMPONENT_DEPENDS := \
	libc \
	heap \
	rboot \
	esp8266 \
	driver \
	esptool \
	fatfs \
	gdbstub \
	spi_flash

#
RELINK_VARS += DISABLE_WIFI
ifeq ($(DISABLE_WIFI),1)
COMPONENT_DEPENDS += esp_no_wifi
else
COMPONENT_DEPENDS += esp_wifi
endif

# rBoot creates ROM images from one or both of these targets
TARGET_OUT_0 := $(BUILD_BASE)/$(APP_NAME)_0.out
TARGET_OUT_1 := $(BUILD_BASE)/$(APP_NAME)_1.out
