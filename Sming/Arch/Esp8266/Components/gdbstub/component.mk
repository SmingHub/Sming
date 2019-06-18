# => APP

CONFIG_VARS			+= ENABLE_GDB
GDBSTUB_BASE		:= $(ARCH_COMPONENTS)/gdbstub
APPCODE				+= $(GDBSTUB_BASE)/appcode
EXTRA_INCDIR		+= $(GDBSTUB_BASE)
ifeq ($(ENABLE_GDB), 1)
	APPCODE			+= $(GDBSTUB_BASE)
	CUSTOM_TARGETS	+= gdb_symbols

# Copy symbols required by GDB into build directory
.PHONY: gdb_symbols
gdb_symbols: $(BUILD_BASE)/bootrom.elf

$(BUILD_BASE)/%.elf:
	$(Q) cp $(ARCH_COMPONENTS)/gdbstub/symbols/$(notdir $@) $@
endif

# Full GDB command line
GDB := trap '' INT; $(GDB) -x $(ARCH_COMPONENTS)/gdbstub/gdbcmds -b $(COM_SPEED_SERIAL) -ex "target remote $(COM_PORT)"

