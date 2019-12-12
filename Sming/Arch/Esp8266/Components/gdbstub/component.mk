COMPONENT_LIBNAME :=

COMPONENT_DEPENDS	:= esp8266

COMPONENT_APPCODE	:= appcode
COMPONENT_INCDIRS	:= \
	. \
	$(ARCH_COMPONENTS)/driver \
	$(ESP_HOME)/xtensa-lx106-elf/include

COMPONENT_DOXYGEN_INPUT := .

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

CACHE_VARS			+= COM_PORT_GDB
DEBUG_VARS			+= COM_SPEED_GDB
COM_PORT_GDB		?= $(COM_PORT)
COM_SPEED_GDB		?= $(COM_SPEED_SERIAL)

# Full GDB command line
DEBUG_VARS			+= GDBSTUB_DIR
GDBSTUB_DIR := $(COMPONENT_PATH)

CACHE_VARS			+= GDB_CMDLINE
GDB_CMDLINE = trap '' INT; $(GDB) -x $(GDBSTUB_DIR)/gdbcmds -b $(COM_SPEED_GDB) -ex "target remote $(COM_PORT_GDB)"

# Swap to alternate serial port pinout
CONFIG_VARS			+= GDB_UART_SWAP
ifeq ($(GDB_UART_SWAP),1)
APP_CFLAGS			+= -DGDB_UART_SWAP=1
endif

#
ifeq ($(USE_NEWLIB),1)
APP_CFLAGS			+= -DGDBSTUB_GDB_PATCHED=0
endif
