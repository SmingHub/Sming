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
application: $(CUSTOM_TARGETS) $(TARGET_BIN)

# $1 -> Linker script
define LinkTarget
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) -T$1 $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
endef

$(TARGET_OUT): $(COMPONENTS_AR)
	$(call LinkTarget,standalone.rom.ld)

	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)

	$(Q)	if [ -f "$(FW_MEMINFO_NEW)" -a -f "$(FW_MEMINFO_OLD)" ]; then \
				$(AWK) -F "|" ' \
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

$(TARGET_BIN): $(TARGET_OUT)
	$(Q) $(ESPTOOL_CMDLINE) elf2image --min-rev 0 --elf-sha256-offset 0xb0 $(flashimageoptions) -o $@ $<

# Application

FLASH_APP_CHUNKS := 0x10000=$(TARGET_BIN)

.PHONY: flashboot
flashboot: $(FLASH_BOOT_LOADER) ##Write just the Bootloader
	$(call WriteFlash,$(FLASH_BOOT_CHUNKS))

.PHONY: flashapp
flashapp: all kill_term ##Write just the application image
	$(call WriteFlash,$(FLASH_APP_CHUNKS))

.PHONY: flash
flash: all partitions kill_term ##Write the boot loader, application image, partition table and (if enabled) SPIFFS image
	$(call WriteFlash,$(FLASH_BOOT_CHUNKS) $(FLASH_APP_CHUNKS) $(FLASH_PARTITION_CHUNKS) $(FLASH_SPIFFS_CHUNKS))
ifeq ($(ENABLE_GDB), 1)
	$(GDB_CMDLINE)
else
	$(TERMINAL)
endif
