###
#
# SMING Application Makefile for Host (Win32/Linux) platform
#
###

# linker flags used to generate the main object file
LDFLAGS = \
	-m32 \
	-Wl,--gc-sections \
	-Wl,-Map=$(basename $@).map

# Executable
TARGET_OUT_0			:= $(FW_BASE)/$(APP_NAME)$(TOOL_EXT)

# Command-line options passed to executable
CACHE_VARS				+= SMING_TARGET_OPTIONS
SMING_TARGET_OPTIONS	?= \
	--flashfile=$(FLASH_BIN) \
	--flashsize=$(SPI_SIZE) \
	--uart=0 \
	--uart=1 \
	--pause=5

# Target definitions

.PHONY: application
application: $(CUSTOM_TARGETS) $(TARGET_OUT_0)

$(TARGET_OUT_0): $(COMPONENTS_AR)
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
	$(Q) $(call WriteFirmwareConfigFile,$@)
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)
	$(Q) cat $(FW_MEMINFO_NEW)


##@Flashing

ifneq ($(DISABLE_SPIFFS), 1)
FLASH_SPIFFS_CHUNKS	:= $(RBOOT_SPIFFS_0)=$(SPIFF_BIN_OUT)
endif

.PHONY: run
run: all ##Run the application image
	$(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)

.PHONY: flashfs
flashfs: $(SPIFF_BIN_OUT) ##Write just the SPIFFS filesystem image
ifeq ($(DISABLE_SPIFFS), 1)
	$(info SPIFFS image creation disabled!)
else
	$(call WriteFlash,$(FLASH_SPIFFS_CHUNKS))
endif

.PHONY: flash
flash: all flashfs ##Write the SPIFFS filesystem image then run the application
ifeq ($(ENABLE_GDB), 1)
	$(GDB_CMDLINE)
else
	$(TARGET_OUT_0) $(SMING_TARGET_OPTIONS)
endif

.PHONY: flashinit
flashinit: | $(FW_BASE) ##Erase all flash memory
	$(info Erasing flash (writing default flash backing file))
	$(Q) $(EraseFlash)

