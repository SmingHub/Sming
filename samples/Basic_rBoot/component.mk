#### overridable rBoot options ####

## use rboot build mode
RBOOT_ENABLED		?= 1

## size of the flash chip
SPI_SIZE			?= 4M

## address of ROM slots 0 & 1
#RBOOT_ROM0_ADDR 	?= 0x002000
RBOOT_ROM1_ADDR 	?= 0x102000

## output file for first rom (.bin will be appended)
#RBOOT_ROM_0 		?= rom0

## these next options only needed when using two rom mode
#RBOOT_ROM_1		?= rom1

## size of the spiffs to create
SPIFF_SIZE			?= 65536

## option to completely disable spiffs
#DISABLE_SPIFFS		?= 1

## flash offsets for spiffs, set if using two rom mode or not on a 4mb flash
## (spiffs location defaults to the mb after the rom slot on 4mb flash)
#RBOOT_SPIFFS_0		?= 0x100000
#RBOOT_SPIFFS_1		?= 0x300000

# Emulate UART 0
ENABLE_HOST_UARTID := 0
