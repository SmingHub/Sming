#
# soc
#

ifeq (esp32,$(SMING_SOC))
    # esp_dport_access_reg_read is added as an undefined symbol because otherwise
    # the linker can ignore dport_access.c as it would no other files depending on any symbols in it.
	SDK_UNDEF_SYMBOLS += \
		esp_dport_access_reg_read
endif
