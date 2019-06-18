
## => Makefile-windows.mk

# Default COM port
COM_PORT	 ?= COM3

# Other tools mappings
KILL_TERM    ?= taskkill.exe -f -im Terminal.exe || exit 0
TERMINAL     ?= $(SDK_TOOLS)/Terminal.exe $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)


## => Makefile-macos.mk

# Default COM port
COM_PORT     ?= /dev/tty.usbserial

# Other tools mappings
KILL_TERM    ?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0
TERMINAL     ?= python -m serial.tools.miniterm $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)


## => Makefile-linux.mk

# Default COM port
COM_PORT     ?= /dev/ttyUSB0

# Other tools mappings
KILL_TERM    ?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0
TERMINAL     ?= python -m serial.tools.miniterm $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)


## => Makefile-bsd.mk

# Default COM port
COM_PORT     ?= /dev/cuaU0

# Other tools mappings
KILL_TERM    ?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0
TERMINAL     ?= python -m serial.tools.miniterm $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)


#
.PHONY: kill_term
kill_term:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)


##@Tools

.PHONY: terminal
terminal: kill_term ##Open the serial terminal
	$(TERMINAL)

