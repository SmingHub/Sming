COMPONENT_LIBNAME :=

COMPONENT_DEPENDS		:= esp8266

CONFIG_VARS				+= SPI_SPEED SPI_MODE SPI_SIZE

# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED				?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE				?= qio
# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE				?= 512K

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
	flashimageoptions	+= -fs 256KB
else ifeq ($(SPI_SIZE), 1M)
	flashimageoptions	+= -fs 1MB
	INIT_BIN_ADDR		:= 0x0fc000
	BLANK_BIN_ADDR		:= 0x0fe000
else ifeq ($(SPI_SIZE), 2M)
	flashimageoptions	+= -fs 2MB
	INIT_BIN_ADDR		:= 0x1fc000
	BLANK_BIN_ADDR		:= 0x1fe000
else ifeq ($(SPI_SIZE), 4M)
	flashimageoptions	+= -fs 4MB
	INIT_BIN_ADDR		:= 0x3fc000
	BLANK_BIN_ADDR		:= 0x3fe000
else
	flashimageoptions	+= -fs 512KB
	INIT_BIN_ADDR		:= 0x07c000
	BLANK_BIN_ADDR		:= 0x04b000
endif

#
FLASH_INIT_CHUNKS		+= $(INIT_BIN_ADDR)=$(SDK_BASE)/bin/esp_init_data_default.bin
FLASH_INIT_CHUNKS		+= $(BLANK_BIN_ADDR)=$(SDK_BASE)/bin/blank.bin

# Default COM port and speed used for flashing
CACHE_VARS				+= COM_PORT_ESPTOOL COM_SPEED_ESPTOOL
COM_PORT_ESPTOOL		?= $(COM_PORT)
COM_SPEED_ESPTOOL		?= $(COM_SPEED)

COMPONENT_SUBMODULES	+= esptool
DEBUG_VARS				+= ESPTOOL
ESPTOOL					:= $(COMPONENT_PATH)/esptool/esptool.py

ESPTOOL_CMDLINE			:= $(ESPTOOL) -p $(COM_PORT_ESPTOOL) -b $(COM_SPEED_ESPTOOL)

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(info WriteFlash $1)
	$(ESPTOOL_CMDLINE) write_flash $(flashimageoptions) $(subst =, ,$1)
endef

# Erase flash memory contents
define EraseFlash
	$(ESPTOOL_CMDLINE) erase_flash
endef
