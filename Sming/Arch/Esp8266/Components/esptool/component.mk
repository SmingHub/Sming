## Flash parameters

# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED			?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE			?= qio
# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE			?= 512K

ifeq ($(SPI_SPEED), 26)
	flashimageoptions	:= -ff 26m
else ifeq ($(SPI_SPEED), 20)
	flashimageoptions	:= -ff 20m
else ifeq ($(SPI_SPEED), 80)
	flashimageoptions	:= -ff 80m
else
	flashimageoptions	:= -ff 40m
endif

ifeq ($(SPI_MODE), qout)
	flashimageoptions	+= -fm qout
else ifeq ($(SPI_MODE), dio)
	flashimageoptions	+= -fm dio
else ifeq ($(SPI_MODE), dout)
	flashimageoptions	+= -fm dout
else
	flashimageoptions	+= -fm qio
endif

ifeq ($(SPI_SIZE), 256K)
	flashimageoptions	+= -fs 2m
else ifeq ($(SPI_SIZE), 1M)
	flashimageoptions	+= -fs 8m
	INIT_BIN_ADDR		:= 0x0fc000
	BLANK_BIN_ADDR		:= 0x0fe000
else ifeq ($(SPI_SIZE), 2M)
	flashimageoptions	+= -fs 16m
	INIT_BIN_ADDR		:= 0x1fc000
	BLANK_BIN_ADDR		:= 0x1fe000
else ifeq ($(SPI_SIZE), 4M)
	flashimageoptions	+= -fs 32m
	INIT_BIN_ADDR		:= 0x3fc000
	BLANK_BIN_ADDR		:= 0x3fe000
else
	flashimageoptions	+= -fs 4m
endif

CFLAGS		+= -DSPIFF_SIZE=$(SPIFF_SIZE)

## => Makefile-windows.mk
ESPTOOL		 ?= $(SDK_TOOLS)/esptool/esptool.py
## => Makefile-bsd.mk
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
## => Makefile-linux.mk
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
## => Makefile-macos.mk
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py


ESPTOOL		:= $(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL)
WRITE_FLASH	:= $(ESPTOOL) write_flash $(flashimageoptions)
ERASE_FLASH	:= $(ESPTOOL) erase_flash

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

