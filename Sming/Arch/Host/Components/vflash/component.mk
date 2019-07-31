COMPONENT_LIBNAME	:=

# Use DD to update FLASH_BIN, which represents the entire flash device contents

DD					:= dd

CACHE_VARS			+= FLASH_BIN
FLASH_BIN			?= $(FW_BASE)/flash.bin

CONFIG_VARS			+= SPI_SIZE
SPI_SIZE			?= 4M

# Options to add when running emulator
CACHE_VARS			+= HOST_FLASH_OPTIONS
HOST_FLASH_OPTIONS	?= --flashfile=$(FLASH_BIN) --flashsize=$(SPI_SIZE)
CLI_TARGET_OPTIONS += $(HOST_FLASH_OPTIONS)

# Write data to flash
# $1 -> Start offset
# $2 -> File containing data to write
define WriteFlashChunk
	$(info WriteFlash $1 -> $2)
	$(Q) if [ ! -f $(FLASH_BIN) ]; then \
		$(EraseFlash); \
	fi
	$(Q) $(DD) if=$2 of=$(FLASH_BIN) obs=1 seek=$$(($1)) conv=notrunc
endef

# Write one or more chunks to flash
# $1 -> List of `Offset=File` chunks
define WriteFlash
	$(foreach c,$1,$(call WriteFlashChunk,$(word 1,$(subst =, ,$c)),$(word 2,$(subst =, ,$c))))
endef

# Reset/create flash backing file
define EraseFlash
	$(DD) if=/dev/zero ibs=1 count=$(SPI_SIZE) | tr "\000" "\377" > $(FLASH_BIN)
endef 
