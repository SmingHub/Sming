#
# Some functions can be extracted separately but only if we discard relocations.
# Generally that means they must be in separate sections, which some functions are not:
#
# 	system_update_cpu_freq
# 	system_get_rst_info
#	spi_flash_erase_sector_check
#	system_get_flash_size_map
#	system_adc_read
#	system_adc_read_fast
#	system_get_vdd33
#	system_get_rst_info
#
NOWIFI_SYMS := \
	system_rtc_mem_read \
	system_rtc_mem_write \
	system_get_time \
	system_restart_core

EXTRA_OBJ := extra.o

# Note *UND* ensures we retain relocations
$(COMPONENT_RULE)$(EXTRA_OBJ): sdk/user_interface.o
	$(Q) $(OBJCOPY) \
		-j *UND* \
		$(addprefix -j .text.,$(NOWIFI_SYMS)) \
		$(foreach f,$(NOWIFI_SYMS),--rename-section .text.$f=.iram.text) \
		sdk/user_interface.o $@

$(COMPONENT_RULE)sdk/user_interface.o:
	mkdir -p sdk
	cd sdk && $(AR) x $(SDK_LIBDIR)/libmain.a user_interface.o

# Remove `user_interface.o` object and weaken our entry point
define NOWIFI_LIBMAIN_COMMANDS
$(Q) $(AR) d $@ user_interface.o
$(Q) $(OBJCOPY) -W call_user_start_local $@

endef
LIBMAIN_COMMANDS += $(NOWIFI_LIBMAIN_COMMANDS)

LIBDIRS += $(COMPONENT_PATH)
EXTRA_LDFLAGS := -Tno.wifi.ld -u call_user_start_local

##

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
