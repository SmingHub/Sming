###
#
# SMING Application Makefile for ESP8266 architecture
#
###

#
LIBS += \
	microc \
	microgcc \
	stdc++ \
	hal \
	$(LIBMAIN)

# linker flags used to generate the main object file
LDFLAGS	+= \
	-nostdlib \
	-u call_user_start \
	-Wl,-static \


.PHONY: application
application: $(CUSTOM_TARGETS) $(FW_FILE_1) $(FW_FILE_2)

LIBDIRS += $(SDK_LIBDIR)

# $1 -> Linker script
define LinkTarget
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) -T$1 $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
endef

$(TARGET_OUT_0): $(COMPONENTS_AR)
	$(call LinkTarget,$(RBOOT_LD_0))

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


$(TARGET_OUT_1): $(COMPONENTS_AR)
	$(call LinkTarget,$(RBOOT_LD_1))


##@Flashing

# If enabled, add the SPIFFS image to the chunks to write
ifneq ($(DISABLE_SPIFFS), 1)
FLASH_SPIFFS_CHUNKS	:= $(RBOOT_SPIFFS_0)=$(SPIFF_BIN_OUT)
FLASH_INIT_CHUNKS	+= $(RBOOT_SPIFFS_0)=$(ARCH_BASE)/Compiler/data/blankfs.bin
endif

.PHONY: flashboot
flashboot: $(RBOOT_BIN) ##Write just the rBoot boot sector
	$(call WriteFlash,$(FLASH_RBOOT_BOOT_CHUNKS))

.PHONY: flashconfig
flashconfig: kill_term ##Erase the rBoot config sector
	$(info Erasing rBoot config sector)
	$(call WriteFlash,$(FLASH_RBOOT_ERASE_CONFIG_CHUNKS))
 
.PHONY: flashapp
flashapp: all kill_term ##Write just the application image
	$(call WriteFlash,$(FLASH_RBOOT_APP_CHUNKS))

.PHONY: flashfs
flashfs: $(SPIFF_BIN_OUT) ##Write just the SPIFFS filesystem image
ifeq ($(DISABLE_SPIFFS), 1)
	$(info SPIFFS image creation disabled!)
else
	$(call WriteFlash,$(FLASH_SPIFFS_CHUNKS))
endif

.PHONY: flash
flash: all kill_term ##Write the rBoot boot sector, application image and (if enabled) SPIFFS image
	$(call WriteFlash,$(FLASH_RBOOT_BOOT_CHUNKS) $(FLASH_RBOOT_APP_CHUNKS) $(FLASH_SPIFFS_CHUNKS))
ifeq ($(ENABLE_GDB), 1)
	$(GDB_CMDLINE)
else
	$(TERMINAL)
endif

.PHONY: flashinit
flashinit: ##Erase your device's flash memory and reset system configuration area to defaults
	$(info Flash init data default and blank data)
	$(info DISABLE_SPIFFS = $(DISABLE_SPIFFS))
	$(EraseFlash)
	$(call WriteFlash,$(FLASH_INIT_CHUNKS))
