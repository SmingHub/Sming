###
#
# SMING Application Makefile for ESP32 architecture
#
###

# linker flags used to generate the main object file
LDFLAGS	+= \
	-nostdlib \
	-Wl,-static

ifeq ($(IDF_VERSION),v5.2)
LDFLAGS += \
	-Wl,--no-warn-rwx-segments
endif

ifdef IDF_TARGET_ARCH_RISCV
	LDFLAGS += \
		-nostartfiles \
		-march=$(ESP32_RISCV_ARCH) \
		--specs=nosys.specs
endif


.PHONY: application
application: $(TARGET_BIN)
ifeq ($(IDF_VERSION),v4.3)
	@printf "\033[47;1;31mWARNING! ESP-IDF 4.3 reached 'End of Life' in December 2023.\033[0m Please upgrade to v5.2.\n"
else ifeq ($(IDF_VERSION),v4.4)
	@printf "\033[47;1;31mWARNING! ESP-IDF 4.4 support has ended in August 2024!\033[0m Please upgrade to v5.2.\n"
else ifeq ($(IDF_VERSION),v5.0)
	@printf "\033[47;1;34mNOTE! ESP-IDF 5.0 not recommended for new designs.\033[0m Please consider upgrading to v5.2.\n"
endif

$(TARGET_OUT): $(COMPONENTS_AR)
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO)
	$(Q) cat $(FW_MEMINFO)

CHIP_REV_MIN := $(CONFIG_$(call ToUpper,$(ESP_VARIANT))_REV_MIN)
ifeq ($(CHIP_REV_MIN),)
CHIP_REV_MIN := 0
endif

ifeq ($(SMING_SOC),esp32c2)
ESPTOOL_EXTRA_ARGS := --flash-mmu-page-size $(CONFIG_MMU_PAGE_MODE)
else
ESPTOOL_EXTRA_ARGS :=
endif

$(TARGET_BIN): $(TARGET_OUT)
	$(Q) $(ESPTOOL_CMDLINE) elf2image --min-rev $(CHIP_REV_MIN) --elf-sha256-offset 0xb0 $(ESPTOOL_EXTRA_ARGS) $(flashimageoptions) -o $@ $<


##@Flashing

.PHONY: bootinfo
bootinfo: $(FLASH_BOOT_LOADER) ##Show bootloader information
	$(info $(FLASH_BOOT_LOADER):)
	$(Q) $(ESPTOOL_CMDLINE) image_info -v2 $(FLASH_BOOT_LOADER)
