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
#SMING_HOME = /opt/Sming/Sming

## COM port parameter is reqruied /opt/Sming/Sming/compiler/ld/eagle.app.v6.cpp.ldto flash firmware correctly.
## Windows: 
# COM_PORT = COM3

# MacOS / Linux:
COM_PORT = /dev/ttyUSB0

# Com port speed
# COM_SPEED	= 576000

# SPIFFs Location
SPIFF_FILES = pic

# Com port speed
COM_SPEED	=  691200 #460800 #691200 #	345600
COM_SPEED_SERIAL = 115200