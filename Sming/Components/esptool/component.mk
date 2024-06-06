COMPONENT_SOC := esp*
COMPONENT_LIBNAME :=

DEBUG_VARS				+= SPI_SPEED SPI_MODE SPI_SIZE
SPI_SPEED				= $(STORAGE_DEVICE_spiFlash_SPEED)
SPI_MODE				= $(STORAGE_DEVICE_spiFlash_MODE)
SPI_SIZE				= $(STORAGE_DEVICE_spiFlash_SIZE)

flashimageoptions		+= -fs $(SPI_SIZE)B -ff $(SPI_SPEED)m -fm $(SPI_MODE)

# Default COM port and speed used for flashing
CACHE_VARS				+= COM_PORT_ESPTOOL COM_SPEED_ESPTOOL
COM_PORT_ESPTOOL		?= $(COM_PORT)
COM_SPEED_ESPTOOL		?= $(COM_SPEED)

COMPONENT_SUBMODULES	+= esptool
DEBUG_VARS				+= ESPTOOL
ESPTOOL					:= $(COMPONENT_PATH)/esptool/esptool.py
ESPTOOL_SUBMODULE		:= $(COMPONENT_PATH)/esptool

$(ESPTOOL): $(ESPTOOL_SUBMODULE)/.submodule

ESPTOOL_CMDLINE := $(PYTHON) $(ESPTOOL) \
	-p $(COM_PORT_ESPTOOL) -b $(COM_SPEED_ESPTOOL) \
	--chip $(SMING_SOC) --before default_reset --after hard_reset

#
# USB serial ports are not available under WSL2,
# but we can use powershell with the regular Windows COM port
# $1 -> Arguments
ifeq ($(WSL_COMPORT_POWERSHELL),1)
ESPTOOL_EXECUTE = powershell.exe -Command "$(ESPTOOL_CMDLINE) $1"
else
ESPTOOL_EXECUTE = $(ESPTOOL_CMDLINE) $1
endif

comma := ,

# Read flash manufacturer ID and determine actual size
define ReadFlashID
	$(info Reading Flash ID)
	$(call ESPTOOL_EXECUTE,flash_id)
endef

# Write file contents to Flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(if $1,\
		$(info WriteFlash $1) \
		$(call ESPTOOL_EXECUTE,write_flash -z $(flashimageoptions) $(subst =, ,$1)) \
	)
endef

define MergeFlash
	$(if $1,\
		$(info MergeFlash $1) \
		$(call ESPTOOL_EXECUTE,merge_bin -o $2 $(flashimageoptions) $(subst =, ,$1)) \
	)
endef

# Verify flash against file contents
# $1 -> List of `Offset=File` chunks
define VerifyFlash
	$(if $1,\
		$(info VerifyFlash $1) \
		$(call ESPTOOL_EXECUTE,verify_flash $(flashimageoptions) $(subst =, ,$1)) \
	)
endef

# Read flash memory into file
# $1 -> `Offset,Size` chunk
# $2 -> Output filename
define ReadFlash
	$(info ReadFlash $1,$2)
	$(call ESPTOOL_EXECUTE,read_flash $(subst $(comma), ,$1) $2)
endef

# Erase a region of Flash
# $1 -> Offset,Size
define EraseFlashRegion
	$(info EraseFlashRegion $1)
	$(call ESPTOOL_EXECUTE,erase_region $(subst $(comma), ,$1))
endef

# Erase flash memory contents
define EraseFlash
	$(call ESPTOOL_EXECUTE,erase_flash)
endef


##@Flashing

.PHONY: esptool
esptool: ##Pass options to esptool, e.g. `make esptool -- --help` or `make esptool image_info` 
	$(Q) $(ESPTOOL_CMDLINE) $(filter-out $@,$(MAKECMDGOALS))

