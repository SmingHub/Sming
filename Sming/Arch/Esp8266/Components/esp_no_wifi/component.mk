define NOWIFI_LIBMAIN_COMMANDS
$(Q) $(AR) d $@ user_interface.o
$(Q) $(OBJCOPY) -W call_user_start_local -W user_start $@

endef
LIBMAIN_COMMANDS += $(NOWIFI_LIBMAIN_COMMANDS)

LIBDIRS += $(COMPONENT_PATH)
EXTRA_LDFLAGS := -Tno.wifi.ld -u user_start -u call_user_start_local
