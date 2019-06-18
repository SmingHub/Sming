# => SMING

RBOOT_BASE		:= $(ARCH_COMPONENTS)/rboot/rboot
SUBMODULES		+= $(RBOOT_BASE)
EXTRA_INCDIR	+= $(RBOOT_BASE) $(RBOOT_BASE)/appcode



# => APP

# rBoot options, overwrite them in the projects Makefile-user.mk
CONFIG_VARS += RBOOT_BIG_FLASH RBOOT_TWO_ROMS RBOOT_RTC_ENABLED RBOOT_GPIO_ENABLED RBOOT_GPIO_SKIP_ENABLED
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

CONFIG_VARS		+= RBOOT_SILENT RBOOT_ROM_0 RBOOT_ROM_1 RBOOT_LD_0 RBOOT_LD_1
RBOOT_SILENT	?= 0
RBOOT_ROM_0		?= rom0
RBOOT_ROM_1		?= rom1
RBOOT_LD_0		?= rboot.rom0.ld
RBOOT_LD_1		?= rom1.ld

#
CONFIG_VARS		+= RBOOT_SPIFFS_0 RBOOT_SPIFFS_1
RBOOT_SPIFFS_0	?= 0x100000
RBOOT_SPIFFS_1	?= 0x300000

# Fixed addresses
ROM_0_ADDR		:= 0x002000
BLANK_BIN_ADDR	:= 0x04b000
INIT_BIN_ADDR	:= 0x07c000

# filenames and options for generating rBoot rom images with esptool2
RBOOT_E2_SECTS		?= .text .data .rodata
RBOOT_E2_USER_ARGS	?= -quiet -bin -boot2

RBOOT_ROM_0		:= $(FW_BASE)/$(RBOOT_ROM_0).bin
RBOOT_ROM_1		:= $(FW_BASE)/$(RBOOT_ROM_1).bin




RBOOT_BASE		:= $(ARCH_COMPONENTS)/rboot
APPCODE			+= $(RBOOT_BASE)/appcode $(RBOOT_BASE)/rboot/appcode
EXTRA_INCDIR	+= $(RBOOT_BASE)/rboot $(RBOOT_BASE)/appcode $(RBOOT_BASE)/rboot/appcode
RBOOT_BIN		:= $(FW_BASE)/rboot.bin
CUSTOM_TARGETS	+= $(RBOOT_BIN)
CFLAGS			+= -DRBOOT_INTEGRATION
# these are exported for use by the rBoot Makefile
export RBOOT_SILENT
export RBOOT_BIG_FLASH
export RBOOT_BUILD_BASE := $(abspath $(BUILD_BASE))
export RBOOT_FW_BASE := $(abspath $(FW_BASE))
export RBOOT_RTC_ENABLED
export RBOOT_GPIO_ENABLED
export RBOOT_GPIO_SKIP_ENABLED
export RBOOT_ROM1_ADDR
export RBOOT_ROM2_ADDR
export SPI_SIZE
export SPI_MODE
export SPI_SPEED
export ESPTOOL2
# multiple roms per 1mb block?
ifeq ($(RBOOT_TWO_ROMS),1)
	# set a define so ota code can choose correct rom
	CFLAGS += -DRBOOT_TWO_ROMS
else
	# eliminate the second rBoot target
	RBOOT_ROM_1 :=
endif
ifeq ($(RBOOT_RTC_ENABLED),1)
	# enable the temporary switch to rom feature
	CFLAGS += -DBOOT_RTC_ENABLED
endif
ifeq ($(RBOOT_GPIO_ENABLED),1)
	CFLAGS += -DBOOT_GPIO_ENABLED
endif
ifeq ($(RBOOT_GPIO_SKIP_ENABLED),1)
	CFLAGS += -DBOOT_GPIO_SKIP_ENABLED
endif
$(RBOOT_BIN):
	$(MAKE) -C $(ARCH_COMPONENTS)/rboot/rboot




# libmain must be modified for rBoot big flash support (just one symbol gets weakened)
ifeq ($(RBOOT_BIG_FLASH),1)
	LIBMAIN			:= main2
	CFLAGS			+= -DBOOT_BIG_FLASH
	LIBMAIN_DST		:= $(BUILD_BASE)/lib$(LIBMAIN).a
	CUSTOM_TARGETS	+= $(LIBMAIN_DST)

$(LIBMAIN_DST): $(LIBMAIN_SRC)
	@echo "OC $@"
	$(Q) $(OBJCOPY) -W Cache_Read_Enable_New $^ $@
endif

LIBS += $(LIBMAIN)





ESPTOOL2	= $(ARCH_TOOLS)/esptool2/esptool2$(TOOL_EXT)

# => esptool2
TOOLS			+= $(ESPTOOL2)
TOOLS_CLEAN		+= esptool2-clean

ESPTOOL2_BASE := $(ARCH_TOOLS)/esptool2
SUBMODULES += $(ESPTOOL2_BASE)
$(ESPTOOL2): | $(ESPTOOL2_BASE)/.submodule
	$(Q) $(call make-tool,$@,$(ESPTOOL2_BASE))

.PHONY: esptool2-clean
esptool2-clean:
	-$(Q) -$(call make-tool,$(ESPTOOL2),$(ESPTOOL2_BASE),clean)








app: $(RBOOT_ROM_0) $(RBOOT_ROM_1)

# => Firmware images
$(RBOOT_ROM_0): $(TARGET_OUT_0)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

$(RBOOT_ROM_1): $(TARGET_OUT_1)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

	
