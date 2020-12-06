###
#
# SMING Application Makefile for ESP8266 architecture
#
###

# Remove object providing millis(), micros() - These functions are defined by the Sming component
DEBUG_VARS += LIBMAIN_COMMANDS

define APP_LIBMAIN_COMMANDS
	$(Q) $(AR) d $@ time.o

endef
LIBMAIN_COMMANDS += $(APP_LIBMAIN_COMMANDS)

# build customized libmain
LIBMAIN_HASH	:= $(call CalculateVariantHash,LIBMAIN_COMMANDS)
LIBMAIN := main-$(LIBMAIN_HASH)

LIBMAIN_SRC = $(SDK_LIBDIR)/libmain.a
LIBMAIN_DST = $(APP_LIBDIR)/lib$(LIBMAIN).a

$(LIBMAIN_DST): $(LIBMAIN_SRC)
	$(info Prepare libmain)
	$(Q) cp $^ $@
	$(LIBMAIN_COMMANDS)

#
LIBS += $(LIBMAIN)

# linker flags used to generate the main object file
LDFLAGS	+= \
	-nostdlib \
	-Wl,-static


.PHONY: application
application: $(CUSTOM_TARGETS) $(FW_FILE_1) $(FW_FILE_2)

# $1 -> Linker script
define LinkTarget
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) -T$1 $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
endef

$(TARGET_OUT_0): $(COMPONENTS_AR) $(LIBMAIN_DST)
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


$(TARGET_OUT_1): $(COMPONENTS_AR) $(LIBMAIN_DST)
	$(call LinkTarget,$(RBOOT_LD_1))


##@Flashing

.PHONY: flashconfig
flashconfig: kill_term ##Erase the rBoot config sector
	$(info Erasing rBoot config sector)
	$(call WriteFlash,$(FLASH_RBOOT_ERASE_CONFIG_CHUNKS))
