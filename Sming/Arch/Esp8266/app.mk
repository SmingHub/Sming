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
application: $(FW_FILE_1) $(FW_FILE_2)

# $1 -> Linker script
define LinkTarget
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) -T$1 $(LDFLAGS) -Wl,--start-group $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) -Wl,--end-group -o $@
endef

$(TARGET_OUT_0): $(COMPONENTS_AR) $(LIBMAIN_DST)
	$(call LinkTarget,$(RBOOT_LD_0))
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO)
	$(Q) cat $(FW_MEMINFO)

$(TARGET_OUT_1): $(COMPONENTS_AR) $(LIBMAIN_DST)
	$(call LinkTarget,$(RBOOT_LD_1))


##@Flashing

.PHONY: flashconfig
flashconfig: kill_term ##Erase the rBoot config sector
	$(info Erasing rBoot config sector)
	$(call WriteFlash,$(FLASH_RBOOT_ERASE_CONFIG_CHUNKS))
