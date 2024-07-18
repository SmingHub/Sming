COMPONENT_LIBNAME :=

# Default COM port
CACHE_VARS		+= COM_PORT
ifeq ($(UNAME),FreeBSD)
COM_PORT		?= /dev/cuaU0
else ifeq ($(UNAME),Darwin)
COM_PORT		?= /dev/tty.usbserial
else ifeq ($(UNAME),Linux)
COM_PORT		?= /dev/ttyUSB0
else ifeq ($(UNAME),Windows)
COM_PORT		?= COM3
endif


# Universal python terminal application
CACHE_VARS		+= COM_OPTS KILL_TERM TERMINAL
COM_OPTS		?= --raw --encoding ascii --rts 0 --dtr 0
KILL_TERM		?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0


##@Tools

.PHONY: kill_term
kill_term:
	$(info Killing Terminal to free $(COM_PORT))
	-$(Q) $(KILL_TERM)

.PHONY: terminal
terminal: kill_term ##Open the serial terminal
ifdef TERMINAL
	$(TERMINAL)
else ifeq ($(WSL_COMPORT_POWERSHELL),1)
	powershell.exe -Command "python -m serial.tools.miniterm $(COM_OPTS) $(COM_PORT) $(COM_SPEED_SERIAL)"
else
	$(PYTHON) -m serial.tools.miniterm $(COM_OPTS) $(COM_PORT) $(COM_SPEED_SERIAL)
endif
