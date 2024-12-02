# pico_int64_ops

ifeq ($(SMING_SOC),rp2040)
WRAPPED_FUNCTIONS += \
	__aeabi_lmul
endif
