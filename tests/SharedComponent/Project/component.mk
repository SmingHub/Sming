## Local build configuration
## Parameters configured here will override default and ENV values.

## SPIFFS options
DISABLE_SPIFFS = 1
# SPIFF_FILES = files

DEBUG_VERBOSE_LEVEL = 3
SPI_SIZE = 4M

# Tell Sming about any Components we need
COMPONENT_DEPENDS := shared-test

.PHONY: execute
execute: run
