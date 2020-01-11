COMPONENT_LIBNAME		:=
COMPONENT_DOXYGEN_INPUT	:= rboot

ifeq ($(SMING_ARCH),Esp8266)
COMPONENT_DEPENDS		:= esp8266
else
RBOOT_EMULATION			:= 1
endif

COMPONENT_SUBMODULES	:= rboot
COMPONENT_INCDIRS		:= rboot appcode rboot/appcode

RBOOT_DIR				:= $(COMPONENT_PATH)

ifndef RBOOT_EMULATION
COMPONENT_SUBMODULES	+= esptool2
DEBUG_VARS				+= ESPTOOL2
ESPTOOL2				:= $(TOOLS_BASE)/esptool2$(TOOL_EXT)
COMPONENT_TARGETS		:= $(ESPTOOL2)
$(COMPONENT_RULE)$(ESPTOOL2):
	$(call MakeTarget,esptool2/Makefile)
endif

# => APP

# rBoot options
CONFIG_VARS				+= RBOOT_RTC_ENABLED RBOOT_GPIO_ENABLED RBOOT_GPIO_SKIP_ENABLED
RBOOT_RTC_ENABLED		?= 0
RBOOT_GPIO_ENABLED		?= 0
# RBOOT_GPIO_SKIP_ENABLED and RBOOT_GPIO_ENABLED cannot be used at the same time.
RBOOT_GPIO_SKIP_ENABLED	?= 0

ifeq ($(RBOOT_GPIO_ENABLED)$(RBOOT_GPIO_SKIP_ENABLED),11)
$(error Cannot enable RBOOT_GPIO_ENABLED and RBOOT_GPIO_SKIP_ENABLED at the same time)
endif

### ROM Addresses ###
# Make sure that your ROM slots and SPIFFS slot(s) do not overlap!

CONFIG_VARS				+= RBOOT_ROM0_ADDR RBOOT_ROM1_ADDR RBOOT_ROM2_ADDR

# Loation of first ROM (default is sector 2 after rboot and rboot config sector)
RBOOT_ROM0_ADDR			?= 0x002000

# The parameter below specifies the location of the second ROM.
# You need a second slot for any kind of firmware update mechanism.
# Leave empty if you don't need a second ROM slot.
RBOOT_ROM1_ADDR			?=

# The parameter below specifies the location of the GPIO ROM.
# It is only used when RBOOT_GPIO_ENABLED = 1
# Note that setting this parameter will only configure rboot.
# The Sming build system does not create a ROM for this slot.
RBOOT_ROM2_ADDR			?=

ifeq ($(RBOOT_GPIO_ENABLED),0)
ifneq ($(RBOOT_ROM2_ADDR),)
$(warning RBOOT_GPIO_ENABLED is 0, RBOOT_ROM2_ADDR will be ignored)
RBOOT_ROM2_ADDR :=
endif
endif

# determine number of roms to generate
ifneq ($(RBOOT_ROM1_ADDR),)
RBOOT_TWO_ROMS := $(shell $(AWK) 'BEGIN { print (ARGV[1] % (1024*1024)) != (ARGV[2] % (1024*1024))}' $(RBOOT_ROM0_ADDR) $(RBOOT_ROM1_ADDR))
else
RBOOT_TWO_ROMS := 0
endif

DEBUG_VARS 				+= RBOOT_TWO_ROMS

# BIGFLASH mode is needed if at least one ROM address exceeds the first 1MB of flash
BIGFLASH_TEST := $(AWK) 'BEGIN { big=0; for(i = 1; i < ARGC; ++i) if(ARGV[i] > 1024*1024) big=1; print big; }'
RBOOT_BIG_FLASH := $(shell $(BIGFLASH_TEST) $(RBOOT_ROM0_ADDR) $(RBOOT_ROM1_ADDR) $(RBOOT_ROM2_ADDR))

DEBUG_VARS 				+= RBOOT_BIG_FLASH


CONFIG_VARS				+= RBOOT_SILENT
RBOOT_SILENT			?= 0

RELINK_VARS				+= RBOOT_ROM_0 RBOOT_ROM_1 RBOOT_LD_TEMPLATE
RBOOT_ROM_0				?= rom0
RBOOT_ROM_1				?= rom1
RBOOT_LD_TEMPLATE		?= $(RBOOT_DIR)/rboot.rom0.ld
RBOOT_LD_0				:= $(BUILD_BASE)/$(RBOOT_ROM_0).ld
RBOOT_LD_1				:= $(BUILD_BASE)/$(RBOOT_ROM_1).ld

#
CONFIG_VARS				+= RBOOT_SPIFFS_0 RBOOT_SPIFFS_1
RBOOT_SPIFFS_0			?= 0x100000
RBOOT_SPIFFS_1			?= 0x300000
APP_CFLAGS				+= -DRBOOT_SPIFFS_0=$(RBOOT_SPIFFS_0)
APP_CFLAGS				+= -DRBOOT_SPIFFS_1=$(RBOOT_SPIFFS_1)

# filenames and options for generating rBoot rom images with esptool2
RBOOT_E2_SECTS			?= .text .text1 .data .rodata
RBOOT_E2_USER_ARGS		?= -quiet -bin -boot2

RBOOT_ROM_0_BIN			:= $(FW_BASE)/$(RBOOT_ROM_0).bin
RBOOT_ROM_1_BIN			:= $(FW_BASE)/$(RBOOT_ROM_1).bin


COMPONENT_APPCODE		:= appcode rboot/appcode $(if $(RBOOT_EMULATION),host)
APP_CFLAGS				+= -DRBOOT_INTEGRATION

# these are exported for use by the rBoot Makefile
export RBOOT_BUILD_BASE	:= $(abspath $(COMPONENT_BUILD_DIR))
export RBOOT_FW_BASE	:= $(abspath $(FW_BASE))
export ESPTOOL2

# multiple roms per 1mb block?
ifeq ($(RBOOT_TWO_ROMS),1)
	# set a define so ota code can choose correct rom
	APP_CFLAGS			+= -DRBOOT_TWO_ROMS
else
	# eliminate the second rBoot target
	RBOOT_ROM_1_BIN		:=
endif

ifeq ($(RBOOT_RTC_ENABLED),1)
	# enable the temporary switch to rom feature
	APP_CFLAGS			+= -DBOOT_RTC_ENABLED
endif

ifeq ($(RBOOT_GPIO_ENABLED),1)
	APP_CFLAGS			+= -DBOOT_GPIO_ENABLED
endif

ifeq ($(RBOOT_GPIO_SKIP_ENABLED),1)
	APP_CFLAGS			+= -DBOOT_GPIO_SKIP_ENABLED
endif

ifndef RBOOT_EMULATION
RBOOT_BIN				:= $(FW_BASE)/rboot.bin
CUSTOM_TARGETS			+= $(RBOOT_BIN)
$(RBOOT_BIN):
	$(Q) $(MAKE) -C $(RBOOT_DIR)/rboot

EXTRA_LDFLAGS			:= -u Cache_Read_Enable_New

ifeq ($(RBOOT_BIG_FLASH),1)
APP_CFLAGS				+= -DBOOT_BIG_FLASH
# rBoot big flash support requires a slightly modified version of libmain (just one symbol gets weakened)
define RBOOT_LIBMAIN_COMMANDS
$(Q) $(OBJCOPY) -W Cache_Read_Enable_New $@

endef
LIBMAIN_COMMANDS += $(RBOOT_LIBMAIN_COMMANDS)
endif

endif # RBOOT_EMULATION

# Define our flash chunks
FLASH_RBOOT_BOOT_CHUNKS				:= 0x00000=$(RBOOT_BIN)
FLASH_RBOOT_APP_CHUNKS				:= $(RBOOT_ROM0_ADDR)=$(RBOOT_ROM_0_BIN)
FLASH_RBOOT_ERASE_CONFIG_CHUNKS		:= 0x01000=$(SDK_BASE)/bin/blank.bin

ifndef RBOOT_EMULATION

# => Automatic linker script generation from template
# $1 -> application target
# $2 -> linker script
# $3 -> ROM address variable (not value!)
define GenerateLinkerScriptTargets
# Mark linker script out-of-date if ROM address differs from previous run
-include $2.config
ifneq ($$(GEN_$3),$$($3))
.PHONY: $2
endif
# Generate linker script from template
$2: $(RBOOT_LD_TEMPLATE)
	$$(info LDGEN $$@)
	$$(Q) $(AWK) 'match($$$$0, /^.*irom0_0_seg[ \t]*:[ \t]*/) { \
			printf "%sorg = 0x40200010 + ($$($3) & 0xFFFFF), len = 1M - ($$($3) & 0xFFFFF) - 0x10\n", substr($$$$0, RSTART, RLENGTH); next \
		} 1 { print $$$$0 }' $$< > $$@
	$$(Q) echo GEN_$3 := $($3) > $2.config
# Make application depend on linker script
$1: $2
endef

$(eval $(call GenerateLinkerScriptTargets,$(TARGET_OUT_0),$(RBOOT_LD_0),RBOOT_ROM0_ADDR))

# => Firmware images
CUSTOM_TARGETS += $(RBOOT_ROM_0_BIN)
$(RBOOT_ROM_0_BIN): $(TARGET_OUT_0)
	$(info ESPTOOL2 $@)
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)
	$(Q) $(call WriteFirmwareConfigFile,$@)

ifneq ($(RBOOT_ROM_1_BIN),)
$(eval $(call GenerateLinkerScriptTargets,$(TARGET_OUT_1),$(RBOOT_LD_1),RBOOT_ROM1_ADDR))

CUSTOM_TARGETS += $(RBOOT_ROM_1_BIN)
$(RBOOT_ROM_1_BIN): $(TARGET_OUT_1)
	$(info ESPTOOL2 $@)
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)
	$(Q) $(call WriteFirmwareConfigFile,$@)

endif

endif # RBOOT_EMULATION
