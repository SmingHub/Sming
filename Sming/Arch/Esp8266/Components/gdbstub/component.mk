COMPONENT_LIBNAME :=

COMPONENT_DEPENDS		:= esp8266

COMPONENT_APPCODE	:= appcode
COMPONENT_INCDIRS	:= . $(ESP8266_COMPONENTS)/driver

ifeq ($(ENABLE_GDB), 1)
COMPONENT_APPCODE	+= .
CUSTOM_TARGETS		+= gdb_symbols
SYMBOLS_SOURCEDIR	:= $(COMPONENT_PATH)/symbols

# Copy symbols required by GDB into build directory
.PHONY: gdb_symbols
gdb_symbols: $(BUILD_BASE)/bootrom.elf

$(BUILD_BASE)/%.elf:
	$(Q) cp $(SYMBOLS_SOURCEDIR)/$(@F) $@
endif

CACHE_VARS			+= COM_PORT_GDB COM_SPEED_GDB
COM_PORT_GDB		?= $(COM_PORT)
COM_SPEED_GDB		?= $(COM_SPEED)

# Full GDB command line
GDBSTUB_DIR := $(COMPONENT_PATH)
GDB_CMDLINE = trap '' INT; $(GDB) -x $(GDBSTUB_DIR)/gdbcmds -b $(COM_SPEED_GDB) -ex "target remote $(COM_PORT_GDB)"
