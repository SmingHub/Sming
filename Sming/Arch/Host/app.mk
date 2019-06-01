###
#
# SMING Application Makefile for Host (Win32/Linux) platform
#
###

##@Building

.PHONY: all
all: libsming checkdirs app ##(default) Build application

#
CONFIG_VARS		+= RBOOT_SPIFFS_0 RBOOT_SPIFFS_1
RBOOT_SPIFFS_0	?= 0x100000
RBOOT_SPIFFS_1	?= 0x300000

# Code compiled with application
APPCODE :=

# Where to look for libraries
LIBDIRS := $(USER_LIBDIR) $(BUILD_BASE) $(ARCH_BASE)/Compiler/ld

# Eventually this will go, but for now we use some Esp8266 code
ESP8266_COMPONENTS	:= $(SMING_HOME)/Arch/Esp8266/Components

EXTRA_INCDIR += $(ARCH_COMPONENTS)/esp_hal/include $(ESP8266_COMPONENTS)/esp8266/include \
				$(ARCH_COMPONENTS)/driver/include $(ESP8266_COMPONENTS)/driver/include \
				$(ARCH_COMPONENTS)/spi_flash/include $(ESP8266_COMPONENTS)/spi_flash/include \
				$(ARCH_COMPONENTS)/libc/include \
				$(ARCH_COMPONENTS)/heap/include \
				$(ARCH_COMPONENTS)/esp_wifi/include

# Macro to make an optional library
# $1 -> The library to make
# $2 -> List of options to add to make command line
define MakeLibrary
	$(Q) $(MAKE) -C $(SMING_HOME) $(patsubst $(SMING_HOME)/%,%,$1) $2
endef

# => SPIFFS
CONFIG_VARS			+= DISABLE_SPIFFS SPIFF_BIN_OUT
DISABLE_SPIFFS		?= 0
SPIFF_BIN_OUT		?= spiff_rom
SPIFF_BIN_OUT		:= $(FW_BASE)/$(SPIFF_BIN_OUT).bin
EXTRA_INCDIR		+= $(ESP8266_COMPONENTS)/spiffs $(COMPONENTS)/spiffs/src
CFLAGS				+= -DRBOOT_SPIFFS_0=$(RBOOT_SPIFFS_0)
CFLAGS				+= -DRBOOT_SPIFFS_1=$(RBOOT_SPIFFS_1)
CFLAGS				+= -D__WORDSIZE=32 # spiffy_host.h
CUSTOM_TARGETS		+= $(SPIFF_BIN_OUT)
APPCODE				+= $(ESP8266_COMPONENTS)/rboot/appcode 

# => LWIP
LWIP_BASE		:= $(ARCH_COMPONENTS)/lwip
LIBS			+= lwip
EXTRA_INCDIR	+= $(LWIP_BASE) $(LWIP_BASE)/lwip/src/include
ifeq ($(UNAME),Windows)
EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/win32/include
else
EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/unix/port/include
endif

# => WPS
CONFIG_VARS	+= ENABLE_WPS
ifeq ($(ENABLE_WPS),1)
   CFLAGS	+= -DENABLE_WPS=1
   LIBS		+= wps
endif

#
LIBS := $(LIBSMING) $(EXTRA_LIBS) $(LIBS) pthread

ifeq ($(UNAME),Windows)
LIBS += wsock32
endif

# linker flags used to generate the main object file
LDFLAGS	= -m32 -Wl,--gc-sections -Wl,-Map=$(basename $@).map

include $(ARCH_BASE)/flash.mk

# Executable
TARGET_OUT_0	:= $(FW_BASE)/$(TARGET)$(TOOL_EXT)

# Command-line options passed to executable
CONFIG_VARS				+= SMING_TARGET_OPTIONS
SMING_TARGET_OPTIONS	?= --flashfile=$(FLASH_BIN) --uart=0 --uart=1 --pause=5
SMING_TARGET_OPTIONS	+= --flashsize=$(SPI_SIZE)

# Full GDB command line
GDB := trap '' INT; $(GDB) -x $(ARCH_COMPONENTS)/gdbstub/gdbcmds --args $(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)

#############
#
# Target definitions
#
#############

include $(SMING_HOME)/modules.mk

# Add APPCODE objects and targets
$(call ScanModules,$(APPCODE))

.PHONY: app
app: $(CUSTOM_TARGETS) $(TARGET_OUT_0)

$(TARGET_OUT_0): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@

	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)

	$(Q) cat $(FW_MEMINFO_NEW)

# recreate it from 0, since you get into problems with same filenames
$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) test ! -f $@ || rm $@
	$(Q) $(AR) rcsP $@ $^

.PHONY: libsming
libsming: $(LIBSMING_DST) ##Build the Sming framework and user libraries
$(LIBSMING_DST):
	$(vecho) "(Re)compiling Sming. Enabled features: $(SMING_FEATURES). This may take some time"
	$(Q) $(MAKE) -C $(SMING_HOME) clean V=$(V) ENABLE_SSL=$(ENABLE_SSL)
	$(Q) $(MAKE) -C $(SMING_HOME) V=$(V) ENABLE_SSL=$(ENABLE_SSL)

.PHONY: rebuild
rebuild: clean all ##Re-build your application


.PHONY: checkdirs
checkdirs: | $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR) $(FW_BASE):
	$(Q) mkdir -p $@

.PHONY: spiff_update
spiff_update: spiff_clean $(SPIFF_BIN_OUT) ##Rebuild the SPIFFS filesystem image

##@Cleaning

.PHONY: spiff_clean
spiff_clean: ##Remove SPIFFS image file
	$(vecho) "Cleaning $(SPIFF_BIN_OUT)"
	$(Q) rm -rf $(SPIFF_BIN_OUT)

# Generating spiffs_bin
$(SPIFF_BIN_OUT):
ifeq ($(DISABLE_SPIFFS), 1)
	$(vecho) "(!) Spiffs support disabled. Remove 'DISABLE_SPIFFS' make argument to enable spiffs."
else
	$(vecho) "Checking for spiffs files"
	$(Q)	if [ -d "$(SPIFF_FILES)" ]; then \
				echo "$(SPIFF_FILES) directory exists. Creating $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) $(SPIFF_FILES) $(SPIFF_BIN_OUT); \
			else \
				echo "No files found in ./$(SPIFF_FILES)."; \
				echo "Creating empty $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) dummy.dir $(SPIFF_BIN_OUT); \
			fi
endif


##@Flashing

.PHONY: run
run: all ##Run the application image
	$(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)
	
.PHONY: flashfs
flashfs: libsming $(SPIFF_BIN_OUT) ##Write just the SPIFFS filesystem image
ifeq ($(DISABLE_SPIFFS), 1)
	$(vecho) "SPIFFS are not enabled!"
else
	$(call WriteFlashChunk,$(RBOOT_SPIFFS_0),$(SPIFF_BIN_OUT))
endif

.PHONY: flash
flash: all flashfs ##Write the SPIFFS filesystem image then run the application
ifeq ($(ENABLE_GDB), 1)
	$(GDB)
else
	$(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)
endif

.PHONY: flashinit
flashinit: ##Erase all flash memory
	$(vecho) "Erase flash (creates default flash backing file)"
	$(ERASE_FLASH)

# Remove build artifacts
.PHONY: clean
clean:
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)
