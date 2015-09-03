## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

#Add your source directories here separated by space
MODULES = app

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
#ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

# MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

# Com port speed
# COM_SPEED	= 115200

# overridable rBoot options
RBOOT_BIG_FLASH = 1
SPIFF_SIZE      = 65536
SPI_SIZE        = 4M
#RBOOT_ROM_0     = rom0
#RBOOT_LD_0      = rom0.ld
#RBOOT_TWO_ROMS  = 0
# these next options only needed when using two rom mode
#RBOOT_SPIFFS_0  = 0x100000
#RBOOT_SPIFFS_1  = 0x300000
#RBOOT_ROM_1     = rom1
#RBOOT_LD_1      = rom1.ld
# option to completely disable spiffs
#DISABLE_SPIFFS  = 1
