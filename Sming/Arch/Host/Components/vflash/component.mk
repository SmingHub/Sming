#
# Flash parameters
#

# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
CONFIG_VARS += SPI_SPEED SPI_MODE SPI_SIZE SPIFF_SIZE
SPI_SIZE		?= 512K

# Use DD to update $(FW_BASE)/flash.bin

DD			:= dd

CONFIG_VARS += FLASH_BIN
FLASH_BIN	?= $(FW_BASE)/flash.bin

# Write data to flash
# $1 -> Start offset
# $2 -> File containing data to write
define WriteFlashChunk
	$(info WriteFlash $1 -> $2)
	$(Q) if [ ! -f $(FLASH_BIN) ]; then \
		$(ERASE_FLASH); \
	fi
	$(Q) $(DD) if=$2 of=$(FLASH_BIN) obs=1 seek=$$(($1)) conv=notrunc
endef

#
ERASE_FLASH	:= $(DD) if=/dev/zero ibs=1 count=$(SPI_SIZE) | tr "\000" "\377" > $(FLASH_BIN) 
