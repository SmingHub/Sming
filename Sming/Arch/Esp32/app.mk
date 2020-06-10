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
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
endef

$(TARGET_OUT): $(COMPONENTS_AR)
	$(call LinkTarget,$(RBOOT_LD_0))

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
		$(Q) $(PYTHON) $(IDF_COMPONENTS_PATH)/esptool_py/esptool/esptool.py --chip esp32 elf2image --flash_mode "dio" --flash_freq "40m" --flash_size "2MB"  --min-rev 0 --elf-sha256-offset 0xb0 -o $@ $<			

##@Flashing

# Partitions
PARTITIONS_CSV = $(BUILD_BASE)/partitions.csv
PARTITIONS_BIN = $(FW_BASE)/partitions.bin

$(BUILD_BASE)/partitions.csv: 
ifeq ($(DISABLE_SPIFFS), 1)
	$(Q) cp $(SDK_PARTITION_PATH)/base.csv $@
else
	$(Q) cp $(SDK_PARTITION_PATH)/spiffs.csv $@ 
endif

$(PARTITIONS_BIN): $(PARTITIONS_CSV)
	$(Q) $(PYTHON) $(IDF_COMPONENTS_PATH)/partition_table/gen_esp32part.py $< $@

.PHONY: partitions
partitions: $(PARTITIONS_BIN) ##Generate partitions table


FLASH_PARTITION_CHUNKS	:= 0x8000=$(PARTITIONS_BIN)

# Application

FLASH_APP_CHUNKS := 0x10000=$(TARGET_BIN)

# If enabled, add the SPIFFS image to the chunks to write
ifneq ($(DISABLE_SPIFFS), 1)
FLASH_SPIFFS_CHUNKS	:= $(SPIFF_START_ADDR)=$(SPIFF_BIN_OUT)
FLASH_INIT_CHUNKS	+= $(SPIFF_START_ADDR)=$(ARCH_BASE)/Compiler/data/blankfs.bin
endif

.PHONY: flashboot
flashboot: $(FLASH_BOOT_LOADER) ##Write just the Bootloader
	$(call WriteFlash,$(FLASH_BOOT_CHUNKS))

.PHONY: flashconfig
 
flashconfig: partitions kill_term ##Write partition config
	$(call WriteFlash,$(FLASH_PARTITION_CHUNKS))
	
flashpartition: flashconfig
 
.PHONY: flashapp
flashapp: all kill_term ##Write just the application image
	$(call WriteFlash,$(FLASH_APP_CHUNKS))

.PHONY: flashfs
flashfs: $(SPIFF_BIN_OUT) ##Write just the SPIFFS filesystem image
ifeq ($(DISABLE_SPIFFS), 1)
	$(info SPIFFS image creation disabled!)
else
	$(call WriteFlash,$(FLASH_SPIFFS_CHUNKS))
endif

.PHONY: flash
flash: all partitions kill_term ##Write the boot loader, application image, partition table and (if enabled) SPIFFS image
	$(call WriteFlash,$(FLASH_BOOT_CHUNKS) $(FLASH_APP_CHUNKS) $(FLASH_PARTITION_CHUNKS) $(FLASH_SPIFFS_CHUNKS))
ifeq ($(ENABLE_GDB), 1)
	$(GDB_CMDLINE)
else
	$(TERMINAL)
endif

.PHONY: flashinit
flashinit: $(ESPTOOL) $(FLASH_INIT_DATA) ##Erase your device's flash memory and reset system configuration area to defaults
	$(info Flash init data default and blank data)
	$(info DISABLE_SPIFFS = $(DISABLE_SPIFFS))
	$(EraseFlash)
	$(call WriteFlash,$(FLASH_INIT_CHUNKS))
