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
	rp2040 \
	uf2 \
	picotool \
	driver \
	gdbstub \
	spi_flash

# ELF and BIN files
DEBUG_VARS		+= TARGET_BIN
TARGET_OUT		= $(BUILD_BASE)/$(APP_NAME).out
TARGET_BIN		= $(FW_BASE)/$(APP_NAME).bin
TARGET_OUT_0	= $(TARGET_OUT)
