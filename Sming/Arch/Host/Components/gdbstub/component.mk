# Full GDB command line
DEBUG_VARS	+= GDBSTUB_DIR
GDBSTUB_DIR := $(COMPONENT_PATH)

CACHE_VARS	+= GDB_CMDLINE
GDB_CMDLINE = trap '' INT; $(GDB) -x $(GDBSTUB_DIR)/gdbcmds --args $(TARGET_OUT_0) $(CLI_TARGET_OPTIONS) --pause -- $(HOST_PARAMETERS)

# LLVM debugger
CACHE_VARS	+= LLDB_CMDLINE
LLDB_CMDLINE = lldb --source $(GDBSTUB_DIR)/lldbcmds $(TARGET_OUT_0) -- $(CLI_TARGET_OPTIONS) --pause $(HOST_PARAMETERS)

##@Tools

.PHONY: lldb
lldb: ##Run the LLVM debugger console
	$(LLDB_CMDLINE)
