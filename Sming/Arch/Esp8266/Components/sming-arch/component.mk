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
	esp8266 \
	driver \
	esp_wifi \
	esptool \
	fatfs \
	gdbstub \
	spi_flash

# => Platform WiFi
COMPONENT_VARS := \
	ENABLE_WPS \
	ENABLE_SMART_CONFIG

# => SSL
ifeq ($(ENABLE_SSL),1)
	COMPONENT_DEPENDS	+= axtls-8266
endif

# => Custom heap
RELINK_VARS				+= ENABLE_CUSTOM_HEAP
ENABLE_CUSTOM_HEAP		?= 0
ifeq ($(ENABLE_CUSTOM_HEAP), 1)
	COMPONENT_DEPENDS	+= custom_heap
else
	LIBMAIN				= main
	LIBMAIN_SRC 		= $(SDK_LIBDIR)/libmain.a
endif

# Must follow custom_heap
COMPONENT_DEPENDS		+= rboot

# => LWIP
COMPONENT_VARS			+= ENABLE_CUSTOM_LWIP
ENABLE_CUSTOM_LWIP		?= 1
ifeq ($(ENABLE_CUSTOM_LWIP), 0)
	COMPONENT_DEPENDS	+= esp-lwip
else ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	COMPONENT_DEPENDS	+= esp-open-lwip
else ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	COMPONENT_DEPENDS	+= lwip2
endif

# => PWM
RELINK_VARS				+= ENABLE_CUSTOM_PWM
ENABLE_CUSTOM_PWM		?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	COMPONENT_DEPENDS	+= pwm_open
else
	LIBS				+= pwm
endif

# rBoot creates ROM images from one or both of these targets
TARGET_OUT_0 := $(BUILD_BASE)/$(APP_NAME)_0.out
TARGET_OUT_1 := $(BUILD_BASE)/$(APP_NAME)_1.out
