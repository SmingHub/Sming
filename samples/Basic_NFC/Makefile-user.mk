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
ESPTOOL2        ?= C:\Espressif\utils\esptool2.exe

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
COM_PORT = COM7
#VERBOSE =1
COM_SPEED	=912400
# MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

# Com port speed
# COM_SPEED	= 115200