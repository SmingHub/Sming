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

# Calculate parameters from SPI_SIZE value (esptool will check validity)
flashimageoptions	+= -fs $(SPI_SIZE)B
FLASH_SIZE			:= $(subst M,*1024K,$(SPI_SIZE))
FLASH_SIZE			:= $(subst K,*1024,$(FLASH_SIZE))
FlashOffset			= $$(($(FLASH_SIZE)-$1))
FLASH_INIT_CHUNKS += \
	$(call FlashOffset,0x5000)=$(SDK_BASE)/bin/blank.bin \
	$(call FlashOffset,0x4000)=$(SDK_BASE)/bin/esp_init_data_default.bin \
	$(call FlashOffset,0x2000)=$(SDK_BASE)/bin/blank.bin

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
