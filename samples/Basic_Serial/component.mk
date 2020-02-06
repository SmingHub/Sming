SPIFF_SIZE      ?= 65536

CUSTOM_TARGETS	:= files/README.md

# Large text file for demo purposes
files/README.md: $(SMING_HOME)/../README.md
	$(Q) mkdir -p $(@D)
	$(Q) cp $< $@

	
# Emulate both serial ports
ENABLE_HOST_UARTID := 0 1
