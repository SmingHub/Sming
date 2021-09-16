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

CHIP_REV_MIN := $(CONFIG_$(call ToUpper,$(ESP_VARIANT))_REV_MIN)
ifeq ($(CHIP_REV_MIN),)
CHIP_REV_MIN := 0
endif

$(TARGET_BIN): $(TARGET_OUT)
	$(Q) $(ESPTOOL_CMDLINE) elf2image --min-rev $(CHIP_REV_MIN) --elf-sha256-offset 0xb0 $(flashimageoptions) -o $@ $<
