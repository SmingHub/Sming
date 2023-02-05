###
#
# SMING Application Makefile for ESP32 architecture
#
###

# linker flags used to generate the main object file
LDFLAGS	+= \
	-nostdlib \
	-Wl,-static


.PHONY: application
application: $(TARGET_BIN)

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
