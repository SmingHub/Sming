
#
MODULES			+= $(ARCH_COMPONENTS)/gdbstub

# Full GDB command line
GDB := trap '' INT; $(GDB) -x $(ARCH_COMPONENTS)/gdbstub/gdbcmds --args $(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)

