# Full GDB command line
GDBSTUB_DIR := $(COMPONENT_PATH)
GDB_CMDLINE = trap '' INT; $(GDB) -x $(GDBSTUB_DIR)/gdbcmds --args $(TARGET_OUT_0) $(CLI_TARGET_OPTIONS) --pause
