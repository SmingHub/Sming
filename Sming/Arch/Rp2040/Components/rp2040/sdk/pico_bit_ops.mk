# pico_bit_ops

ifeq ($(SMING_SOC),rp2040)
WRAPPED_FUNCTIONS += \
	__clzsi2 \
	__clzdi2 \
	__ctzsi2 \
	__ctzdi2 \
	__popcountsi2 \
	__popcountdi2
endif
