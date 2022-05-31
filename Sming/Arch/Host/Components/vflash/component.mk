COMPONENT_LIBNAME	:=

# Use DD to update FLASH_BIN, which represents the entire flash device contents

DD					:= dd

CACHE_VARS			+= FLASH_BIN
FLASH_BIN			?= $(FW_BASE)/flash.bin

DEBUG_VARS			+= SPI_SIZE
SPI_SIZE			= $(STORAGE_DEVICE_spiFlash_SIZE)

# Options to add when running emulator
CACHE_VARS			+= HOST_FLASH_OPTIONS
HOST_FLASH_OPTIONS	?= --flashfile=$(FLASH_BIN) --flashsize=$(SPI_SIZE)
override CLI_TARGET_OPTIONS += $(HOST_FLASH_OPTIONS)

# Virtual flasher tool
VFLASH := $(PYTHON) $(COMPONENT_PATH)/vflash.py $(FLASH_BIN) $(STORAGE_DEVICE_spiFlash_SIZE_BYTES)

# Write one or more chunks to flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(if $1,$(Q) $(VFLASH) write-chunks $1)
endef

# Verify one or more chunks against flash content
# $1 -> List of `Offset=File` chunks
define VerifyFlash
	@echo VerifyFlash not implemented for Host
endef

# Read flash manufacturer ID and determine actual size
define ReadFlashID
	$(info ReadFlashID: Flash backing file "$(FLASH_BIN)", size $(SPI_SIZE))
endef

# Read flash memory into file
# $1 -> `Offset,Size` chunk
# $2 -> Output filename
define ReadFlash
	$(info ReadFlash $1,$2)
	$(Q) $(VFLASH) read-chunks $1=$2
endef

# Erase a region of Flash
# $1 -> List of `Offset,Size` chunks
define EraseFlashRegion
	$(Q) $(VFLASH) fill-regions $1
endef

# Reset/create flash backing file
define EraseFlash
	$(Q) $(VFLASH) erase
endef
