
IDF_TARGET ?= esp32

COMPONENT_SRCDIRS := \
	$(ARCH_CORE) $(call ListAllSubDirs,$(ARCH_CORE)) \
	$(ARCH_BASE)/Platform \
	$(ARCH_BASE)/Services/Profiling

COMPONENT_INCDIRS := \
	$(ARCH_BASE) \
	$(ARCH_CORE) \
	$(ARCH_COMPONENTS)

COMPONENT_DEPENDS := \
	libc \
	spi_flash \
	driver \
	heap \
	esp32 \
	gdbstub \
	esptool \
	bt

# ELF and BIN files
DEBUG_VARS		+= TARGET_BIN TARGET_OUT
TARGET_OUT		= $(BUILD_BASE)/$(APP_NAME).out
TARGET_BIN		= $(FW_BASE)/$(APP_NAME).bin
TARGET_OUT_0	= $(TARGET_OUT) 
