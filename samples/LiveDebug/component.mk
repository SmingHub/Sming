DISABLE_SPIFFS = 1

ENABLE_GDB = 1

ENABLE_GDB_CONSOLE ?= 1

ifeq ($(ENABLE_GDB_CONSOLE), 1)
USER_CFLAGS := -DGDBSTUB_ENABLE_SYSCALL

all:
	$(warning WARNING! Enabling the GDB console may interfere with visual debuggers, like eclipse)
	$(warning If required, please build with `make ENABLE_GDB_CONSOLE=0`)
endif


# Emulate UART 0
ENABLE_HOST_UARTID := 0
