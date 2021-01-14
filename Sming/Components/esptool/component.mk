COMPONENT_LIBNAME :=

CONFIG_VARS				+= SPI_SPEED SPI_MODE SPI_SIZE

# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED				?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE				?= dio
# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE				?= 1M

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
ifeq ($(SPI_SIZE),detect)
flashimageoptions	+= -fs detect
else
flashimageoptions	+= -fs $(SPI_SIZE)B
endif
FLASH_SIZE			:= $(subst M,*1024K,$(SPI_SIZE))
FLASH_SIZE			:= $(subst K,*1024,$(FLASH_SIZE))
FlashOffset			= $$(($(FLASH_SIZE)-$1))
BLANK_BIN			:= $(COMPONENT_PATH)/blank.bin

# Default COM port and speed used for flashing
CACHE_VARS				+= COM_PORT_ESPTOOL COM_SPEED_ESPTOOL
COM_PORT_ESPTOOL		?= $(COM_PORT)
COM_SPEED_ESPTOOL		?= $(COM_SPEED)

COMPONENT_SUBMODULES	+= esptool
DEBUG_VARS				+= ESPTOOL
ESPTOOL					:= $(COMPONENT_PATH)/esptool/esptool.py
ESPTOOL_SUBMODULE		:= $(COMPONENT_PATH)/esptool

$(ESPTOOL): $(ESPTOOL_SUBMODULE)/.submodule

ifeq ($(SMING_ARCH),Esp8266)
ESP_CHIP := esp8266
else ifeq ($(SMING_ARCH),Esp32)
ESP_CHIP := esp32
else ifeq ($(MAKE_DOCS),)
$(error esptool unsupported arch: $(SMING_ARCH))
endif

ESPTOOL_CMDLINE := $(PYTHON) $(ESPTOOL) \
	-p $(COM_PORT_ESPTOOL) -b $(COM_SPEED_ESPTOOL) \
	--chip $(ESP_CHIP) --before default_reset --after hard_reset

#
# USB serial ports are not available under WSL2,
# but we can use powershell with the regular Windows COM port
# $1 -> Arguments
ifdef WSL_ROOT
ESPTOOL_EXECUTE = powershell.exe -Command "$(ESPTOOL_CMDLINE) $1"
else
ESPTOOL_EXECUTE = $(ESPTOOL_CMDLINE) $1
endif

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(if $1,\
		$(info WriteFlash $1) \
		$(call ESPTOOL_EXECUTE,write_flash -z $(flashimageoptions) $(subst =, ,$1)) \
	)
endef

# Erase flash memory contents
define EraseFlash
	$(call ESPTOOL_EXECUTE,erase_flash)
endef
