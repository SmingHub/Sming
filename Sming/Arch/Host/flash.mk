#
# Flash parameters
#

# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE				?= 512K

ifeq ($(SPI_SIZE), 256K)
	SPIFF_SIZE			?= 131072  #128K
else ifeq ($(SPI_SIZE), 1M)
	SPIFF_SIZE			?= 524288  #512K
else ifeq ($(SPI_SIZE), 2M)
	SPIFF_SIZE			?= 524288  #512K
else ifeq ($(SPI_SIZE), 4M)
	SPIFF_SIZE			?= 524288  #512K
else
	SPIFF_SIZE			?= 196608  #192K
endif

CFLAGS		+= -DSPIFF_SIZE=$(SPIFF_SIZE)


# Use DD to update $(FW_BASE)/flash.bin

DD			:= dd

FLASH_BIN	:= flash.bin

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
