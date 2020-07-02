COMPONENT_SRCDIRS := src proto
COMPONENT_INCDIRS := $(COMPONENT_SRCDIRS) 
COMPONENT_DEPENDS := nanopb

# Architecture of the device where the hosted service will be flashed
HOSTED_ARCH ?= Esp8266

HOSTED_APP_DIR := $(COMPONENT_PATH)/app

RELINK_VARS = ENABLE_HOSTED

##@Building

hosted-app: ##Builds the hosted service firmware
	$(MAKE) -C $(HOSTED_APP_DIR) SMING_ARCH=$(HOSTED_ARCH)

##@Flashing

hosted-flash: ##Flashes the hosted service firmware to an actual device
	$(MAKE) -C $(HOSTED_APP_DIR) flash SMING_ARCH=$(HOSTED_ARCH)
	
hosted-flashapp: ##Flashes only the hosted service app
	$(MAKE) -C $(HOSTED_APP_DIR) flashapp SMING_ARCH=$(HOSTED_ARCH)

