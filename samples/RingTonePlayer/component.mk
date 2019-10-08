SPIFF_SIZE=0x40000
GDB_UART_SWAP=1
ARDUINO_LIBRARIES := \
	ToneGenerator \
	RingTone \
	ArduinoJson6
	
COMPONENT_DEPENDS := \
	malloc_count

ifeq ($(SMING_ARCH),Host)
COMPONENT_SRCDIRS += host
endif
