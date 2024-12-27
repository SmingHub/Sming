# pico_divider

ifeq ($(SMING_SOC),rp2040)
WRAPPED_FUNCTIONS += \
	__aeabi_idiv \
	__aeabi_idivmod \
	__aeabi_ldivmod \
	__aeabi_uidiv \
	__aeabi_uidivmod \
	__aeabi_uldivmod
endif
