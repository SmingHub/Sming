define NOWIFI_LIBMAIN_COMMANDS
$(Q) $(AR) d $@ user_interface.o
$(Q) $(OBJCOPY) -W call_user_start_local -W user_start $@

endef
LIBMAIN_COMMANDS += $(NOWIFI_LIBMAIN_COMMANDS)

LIBDIRS += $(COMPONENT_PATH)
EXTRA_LDFLAGS := -Tno.wifi.ld -u user_start -u call_user_start_local


# Disassemble archive contents into a directory, one .asm file per object
# $1 -> Path to archive
# $2 -> Output directory
define DisassembleArchive
	$(Q) mkdir -p $2
	$(Q) cd $2 && $(AR) xo $1
	$(foreach obj,$(shell $(AR) t $1),
		$(vecho) Creating $2/$(obj:.o=.asm)
		$(Q) $(OBJDUMP) -s -S -g -e -t -r -d $2/$(obj) > $2/$(obj:.o=.asm)
		$(Q) rm $2/$(obj)
	)
endef

SDK_LIBLIST ?= main phy pp wpa
sdk-disassemble: ## Disassemble SDK libraries for inspection
	$(foreach lib,$(SDK_LIBLIST), \
		$(call DisassembleArchive,$(SDK_LIBDIR)/lib$(lib).a,sdk/$(lib)) \
	)
