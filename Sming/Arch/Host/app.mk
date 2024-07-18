###
#
# SMING Application Makefile for Host (Win32/Linux) platform
#
###

# linker flags used to generate the main object file
ifneq ($(BUILD64),1)
LDFLAGS += -m32
endif

ifeq ($(ENABLE_SANITIZERS),1)
LDFLAGS += $(foreach s,$(SANITIZERS),-fsanitize=$s)
endif

# Executable
TARGET_OUT_0			:= $(FW_BASE)/$(APP_NAME)$(TOOL_EXT)

# Hosted Settings
ifneq ($(ENABLE_HOSTED),)
COMPONENTS_AR := \
	$(CMP_Hosted-Lib_TARGETS) \
	$(COMPONENTS_AR)
endif

# Target definitions

.PHONY: application
application: $(TARGET_OUT_0)

COMMA=,
WL=-Wl,

ifeq ($(UNAME),Darwin)
LDFLAGS_FILTERED = $(filter-out $(addprefix $(WL),-EL --gc-sections -wrap%),$(LDFLAGS))
LDFLAGS2 = $(patsubst $(WL)-Map=%,$(WL)-map$(COMMA)%,$(LDFLAGS_FILTERED)) \
	$(WL)-w \
	$(WL)-undefined,suppress \
	$(WL)-flat_namespace
else
LDSTARTGROUP := $(WL)--start-group
LDENDGROUP := $(WL)--end-group
LDFLAGS2 = $(LDFLAGS)
endif

$(TARGET_OUT_0): $(COMPONENTS_AR)
ifdef CLANG_TIDY
	$(info Skipping link step for clang-tidy)
else
	$(info $(notdir $(PROJECT_DIR)): Linking $@)
	$(Q) $(LD) $(addprefix -L,$(LIBDIRS)) $(LDFLAGS2) $(LDSTARTGROUP) $(COMPONENTS_AR) $(addprefix -l,$(LIBS)) $(LDENDGROUP) -o $@
	$(Q) $(call WriteFirmwareConfigFile,$@)
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO)
	$(Q) cat $(FW_MEMINFO)
endif

##@Tools

.PHONY: valgrind
valgrind:
	$(Q) RUN_COMMAND_PREFIX="valgrind --track-origins=yes --leak-check=full" $(MAKE) run

RUN_SCRIPT := $(FW_BASE)/run.sh

.PHONY: run
run: all $(RUN_SCRIPT) ##Run the application image
	$(Q) $(RUN_SCRIPT)

$(RUN_SCRIPT)::
	$(Q) echo '#!/bin/bash' > $@; \
	$(foreach id,$(ENABLE_HOST_UARTID),echo '$(call RunHostTerminal,$(id))' >> $@;) \
	echo '$(RUN_COMMAND_PREFIX) $(TARGET_OUT_0) $(CLI_TARGET_OPTIONS) -- $(HOST_PARAMETERS)' >> $@; \
	chmod a+x $@

##@Flashing

.PHONY: flashconfig
flashconfig: kill_term ##Erase the rBoot config sector
	$(info Erasing rBoot config sector)
	$(call WriteFlash,$(FLASH_RBOOT_ERASE_CONFIG_CHUNKS))
