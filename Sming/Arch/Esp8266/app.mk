###
#
# SMING Application Makefile for ESP8266 architecture
#
###

##@Building

.PHONY: all
all: libsming checkdirs app ##(default) Build application



# Code compiled with application
APPCODE :=

# EXTRA_INCDIR += ... component.mk

# Macro to make an optional library
# $1 -> The library to make
# $2 -> List of options to add to make command line
define MakeLibrary
	$(Q) $(MAKE) -C $(SMING_HOME) $(patsubst $(SMING_HOME)/%,%,$1) $2
endef

#
LIBS := microc microgcc hal phy pp net80211 wpa $(LIBSMING) crypto smartconfig $(EXTRA_LIBS) $(LIBS)

# linker flags used to generate the main object file
LDFLAGS	= -nostdlib -u call_user_start -u Cache_Read_Enable_New -u custom_crash_callback \
			-Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map -Wl,-wrap,system_restart_local 

include $(ARCH_BASE)/flash.mk

TARGET_OUT_0 := $(BUILD_BASE)/$(TARGET)_0.out
TARGET_OUT_1 := $(BUILD_BASE)/$(TARGET)_1.out

#############
#
# Target definitions
#
#############

include $(SMING_HOME)/modules.mk

# Add APPCODE objects and targets
$(call ScanModules,$(APPCODE))

.PHONY: app
app: $(CUSTOM_TARGETS) $(RBOOT_ROM_0) $(RBOOT_ROM_1) $(FW_FILE_1) $(FW_FILE_2)

# => Firmware images
$(RBOOT_ROM_0): $(TARGET_OUT_0)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

$(RBOOT_ROM_1): $(TARGET_OUT_1)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

$(TARGET_OUT_0): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(ARCH_BASE)/Compiler/ld \
		-T$(RBOOT_LD_0) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@

	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)

	$(Q)	if [ -f "$(FW_MEMINFO_NEW)" -a -f "$(FW_MEMINFO_OLD)" ]; then \
				awk -F "|" ' \
					FILENAME == "$(FW_MEMINFO_OLD)" { \
						arr[$$1]=$$5 \
					} \
					FILENAME == "$(FW_MEMINFO_NEW)" { \
					if (arr[$$1] != $$5) { \
						printf "%s%s%+d%s", substr($$0, 1, length($$0) - 1)," (",$$5 - arr[$$1],")\n" \
					} else { \
						print $$0 \
					} \
				}' $(FW_MEMINFO_OLD) $(FW_MEMINFO_NEW); \
			elif [ -f "$(FW_MEMINFO_NEW)" ]; then \
			  cat $(FW_MEMINFO_NEW); \
			fi


$(TARGET_OUT_1): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(ARCH_BASE)/Compiler/ld \
		-T$(RBOOT_LD_1) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@

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

##@Flashing

.PHONY: flashboot
flashboot: libsming $(RBOOT_BIN) ##Write just the rBoot boot sector
	$(WRITE_FLASH) 0x00000 $(RBOOT_BIN)

.PHONY: flashconfig
flashconfig: kill_term ##Erase the rBoot config sector
	$(vecho) "Erasing rBoot config sector"
	$(WRITE_FLASH) 0x01000 $(SDK_BASE)/bin/blank.bin 
 
.PHONY: flashapp
flashapp: all kill_term ##Write just the application image
	$(WRITE_FLASH) $(ROM_0_ADDR) $(RBOOT_ROM_0)

# flashes rboot and first rom
FLASH_CHUNKS := 0x00000 $(RBOOT_BIN)
FLASH_CHUNKS += $(ROM_0_ADDR) $(RBOOT_ROM_0)
ifneq ($(DISABLE_SPIFFS), 1)
	FLASH_CHUNKS += $(RBOOT_SPIFFS_0) $(SPIFF_BIN_OUT)
endif

.PHONY: flash
flash: all kill_term ##Write the rBoot boot sector, application image and (if enabled) SPIFFS image
	$(WRITE_FLASH) $(FLASH_CHUNKS)
ifeq ($(ENABLE_GDB), 1)
	$(GDB)
else
	$(TERMINAL)
endif

# Wipe flash
FLASH_INIT_CHUNKS := $(INIT_BIN_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin
FLASH_INIT_CHUNKS += $(BLANK_BIN_ADDR) $(SDK_BASE)/bin/blank.bin
ifneq ($(DISABLE_SPIFFS), 1)
	FLASH_INIT_CHUNKS += $(RBOOT_SPIFFS_0) $(ARCH_BASE)/Compiler/data/blankfs.bin
endif

.PHONY: flashinit
flashinit: ##Erase your device's flash memory and reset system configuration area to defaults
	$(vecho) "Flash init data default and blank data."
	$(vecho) "DISABLE_SPIFFS = $(DISABLE_SPIFFS)"
	$(ERASE_FLASH)
	$(WRITE_FLASH) $(FLASH_INIT_CHUNKS)

##@Cleaning

.PHONY: clean
clean: ##Remove all generated build files
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)
