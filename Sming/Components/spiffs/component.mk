## SPIFFS library
COMPONENT_SUBMODULES	:= spiffs
COMPONENT_SRCDIRS		:= . spiffs/src
COMPONENT_INCDIRS		:= . spiffs/src


## Spiffy tool

SPIFFY				:= $(TOOLS_BASE)/spiffy$(TOOL_EXT)
COMPONENT_TARGETS	+= $(SPIFFY)
$(COMPONENT_RULE)$(SPIFFY):
	$(call MakeTarget,spiffy/Makefile)


## Application

# This controls filesystem generation, it doesn't actually disable SPIFFS support in the application
CACHE_VARS			+= DISABLE_SPIFFS
DISABLE_SPIFFS		?= 0

CACHE_VARS			+= SPIFF_FILES SPIFF_BIN
SPIFF_FILES			?= files
SPIFF_BIN			?= spiff_rom
SPIFF_BIN_OUT		:= $(FW_BASE)/$(SPIFF_BIN).bin
CUSTOM_TARGETS		+= $(SPIFF_BIN_OUT)

CONFIG_VARS	+= SPIFF_SIZE
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
APP_CFLAGS				+= -DSPIFF_SIZE=$(SPIFF_SIZE)

##@Building

.PHONY: spiffs-image-update
spiffs-image-update: spiffs-image-clean $(SPIFF_BIN_OUT) ##Rebuild the SPIFFS filesystem image

##@Cleaning

.PHONY: spiffs-image-clean
spiffs-image-clean: ##Remove SPIFFS image file
	$(info Cleaning $(SPIFF_BIN_OUT))
	$(Q) rm -f $(SPIFF_BIN_OUT)

# Generating spiffs_bin
$(SPIFF_BIN_OUT): $(SPIFFY)
ifeq ($(DISABLE_SPIFFS), 1)
	$(info (!) Spiffs support disabled. Remove 'DISABLE_SPIFFS' make argument to enable spiffs.)
else
	$(Q) mkdir -p $(dir $(SPIFF_BIN_OUT))
	$(info Checking for spiffs files)
	$(Q)	if [ -d "$(SPIFF_FILES)" ]; then \
				echo "$(SPIFF_FILES) directory exists. Creating $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) $(SPIFF_FILES) $(SPIFF_BIN_OUT); \
			else \
				echo "No files found in ./$(SPIFF_FILES)."; \
				echo "Creating empty $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) dummy.dir $(SPIFF_BIN_OUT); \
			fi
endif
