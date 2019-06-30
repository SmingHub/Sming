COMPONENT_LIBNAME :=

# Default COM port
CACHE_VARS		+= COM_PORT
ifeq ($(UNAME),FreeBSD)
COM_PORT		?= /dev/cuaU0
else ifeq ($(UNAME),MacOS)
COM_PORT		?= /dev/tty.usbserial
else ifeq ($(UNAME),Linux)
COM_PORT		?= /dev/ttyUSB0
else ifeq ($(UNAME),Windows)
COM_PORT		?= COM3
endif


# Universal python terminal application
KILL_TERM		?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0
TERMINAL		?= python -m serial.tools.miniterm $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)

# Alternative for Windows
#KILL_TERM		?= taskkill.exe -f -im Terminal.exe || exit 0
#TERMINAL		?= $(SDK_TOOLS)/Terminal.exe $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)


##@Tools

.PHONY: kill_term
kill_term:
	$(info Killing Terminal to free $(COM_PORT))
	-$(Q) $(KILL_TERM)

.PHONY: terminal
terminal: kill_term ##Open the serial terminal
	$(TERMINAL)
