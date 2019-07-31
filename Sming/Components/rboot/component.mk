COMPONENT_LIBNAME		:=

ifeq ($(SMING_ARCH),Esp8266)
COMPONENT_DEPENDS		:= esp8266
else
RBOOT_EMULATION			:= 1
endif

COMPONENT_SUBMODULES	:= rboot
COMPONENT_INCDIRS		:= rboot appcode rboot/appcode

ifndef RBOOT_EMULATION
COMPONENT_SUBMODULES	+= esptool2
DEBUG_VARS				+= ESPTOOL2
ESPTOOL2				:= $(TOOLS_BASE)/esptool2$(TOOL_EXT)
COMPONENT_TARGETS		:= $(ESPTOOL2)
$(COMPONENT_RULE)$(ESPTOOL2):
	$(call MakeTarget,esptool2/Makefile)
endif

# => APP

# rBoot options, overwrite them in the projects Makefile-user.mk
CONFIG_VARS				+= RBOOT_BIG_FLASH RBOOT_TWO_ROMS RBOOT_RTC_ENABLED RBOOT_GPIO_ENABLED RBOOT_GPIO_SKIP_ENABLED
RBOOT_BIG_FLASH			?= 1
RBOOT_TWO_ROMS			?= 0
RBOOT_RTC_ENABLED		?= 0
RBOOT_GPIO_ENABLED		?= 0
# RBOOT_GPIO_SKIP_ENABLED and RBOOT_GPIO_ENABLED cannot be used at the same time.
RBOOT_GPIO_SKIP_ENABLED	?= 0

ifeq ($(RBOOT_GPIO_ENABLED)$(RBOOT_GPIO_SKIP_ENABLED),11)
$(error "Cannot enable RBOOT_GPIO_ENABLED and RBOOT_GPIO_SKIP_ENABLED at the same time)
endif

### ROM Addresses ###
# The parameter below specifies the location of the second rom.
# This parameter is used only when RBOOT_BIG_FLASH = 1 
# BOOT_ROM1_ADDR = 0x200000

# The parameter below specifies the location of the GPIO ROM.
# This parameter is used only when RBOOT_GPIO_ENABLED = 1
# If you use two SPIFFS make sure that this address is minimum
# RBOOT_SPIFFS_1 + SPIFF_SIZE 
# BOOT_ROM2_ADDR = 0x310000

CONFIG_VARS				+= RBOOT_SILENT
RBOOT_SILENT			?= 0

RELINK_VARS				+= RBOOT_ROM_0 RBOOT_ROM_1 RBOOT_LD_0 RBOOT_LD_1
RBOOT_ROM_0				?= rom0
RBOOT_ROM_1				?= rom1
RBOOT_LD_0				?= rboot.rom0.ld
RBOOT_LD_1				?= rom1.ld

#
CONFIG_VARS				+= RBOOT_SPIFFS_0 RBOOT_SPIFFS_1
RBOOT_SPIFFS_0			?= 0x100000
RBOOT_SPIFFS_1			?= 0x300000
APP_CFLAGS				+= -DRBOOT_SPIFFS_0=$(RBOOT_SPIFFS_0)
APP_CFLAGS				+= -DRBOOT_SPIFFS_1=$(RBOOT_SPIFFS_1)

# Fixed addresses
ROM_0_ADDR				:= 0x002000

# filenames and options for generating rBoot rom images with esptool2
RBOOT_E2_SECTS			?= .text .data .rodata
RBOOT_E2_USER_ARGS		?= -quiet -bin -boot2

RBOOT_ROM_0_BIN			:= $(FW_BASE)/$(RBOOT_ROM_0).bin
RBOOT_ROM_1_BIN			:= $(FW_BASE)/$(RBOOT_ROM_1).bin


COMPONENT_APPCODE		:= appcode rboot/appcode
APP_CFLAGS				+= -DRBOOT_INTEGRATION

# these are exported for use by the rBoot Makefile
CONFIG_VARS				+= RBOOT_ROM1_ADDR RBOOT_ROM2_ADDR
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
RBOOT_DIR := $(COMPONENT_PATH)/rboot
$(RBOOT_BIN):
	$(Q) $(MAKE) -C $(RBOOT_DIR)

# rBoot big flash support requires a slightly modified version of libmain (just one symbol gets weakened)
# Note that LIBMAIN/LIBMAIN_SRC changes depends on whether we're using a custom heap allocator
LIBMAIN_RBOOT			:= $(LIBMAIN)-rboot
LIBMAIN_RBOOT_DST		:= $(APP_LIBDIR)/lib$(LIBMAIN_RBOOT).a
CUSTOM_TARGETS			+= $(LIBMAIN_RBOOT_DST)
COMPONENT_LDFLAGS		:= -u Cache_Read_Enable_New

$(LIBMAIN_RBOOT_DST): $(LIBMAIN_SRC)
	@echo "OC $@"
	$(Q) $(OBJCOPY) -W Cache_Read_Enable_New $^ $@

ifeq ($(RBOOT_BIG_FLASH),1)
LIBMAIN					:= $(LIBMAIN_RBOOT)
APP_CFLAGS				+= -DBOOT_BIG_FLASH
endif

endif # RBOOT_EMULATION

# Define our flash chunks
FLASH_RBOOT_BOOT_CHUNKS				:= 0x00000=$(RBOOT_BIN)
FLASH_RBOOT_APP_CHUNKS				:= $(ROM_0_ADDR)=$(RBOOT_ROM_0_BIN)
FLASH_RBOOT_ERASE_CONFIG_CHUNKS		:= 0x01000=$(SDK_BASE)/bin/blank.bin

ifndef RBOOT_EMULATION
# => Firmware images
CUSTOM_TARGETS += $(RBOOT_ROM_0_BIN)
$(RBOOT_ROM_0_BIN): $(TARGET_OUT_0)
	$(info ESPTOOL2 $@)
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)
	$(Q) $(call WriteFirmwareConfigFile,$@)

CUSTOM_TARGETS += $(RBOOT_ROM_1_BIN)
$(RBOOT_ROM_1_BIN): $(TARGET_OUT_1)
	$(info ESPTOOL2 $@)
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)
	$(Q) $(call WriteFirmwareConfigFile,$@)
endif
