#
# soc
#

ifeq (esp32,$(SMING_SOC))
	SDK_UNDEF_SYMBOLS += \
		esp_dport_access_reg_read
endif
