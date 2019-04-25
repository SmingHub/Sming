###
#
# SMING Application Makefile for ESP8266 platform
#
###

ifndef ESP_HOME
$(error ESP_HOME variable is not set to a valid directory.)
endif

.PHONY: all checkdirs app spiff_update spiff_clean clean kill_term terminal gdb
all: libsming checkdirs app

# rBoot options, overwrite them in the projects Makefile-user.mk
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

RBOOT_SILENT	?= 0
RBOOT_ROM_0		?= rom0
RBOOT_ROM_1		?= rom1
RBOOT_LD_0		?= rboot.rom0.ld
RBOOT_LD_1		?= rom1.ld

#
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

# Code compiled with application
APPCODE :=

EXTRA_INCDIR += $(PLATFORM_COMPONENTS) \
				$(PLATFORM_COMPONENTS)/esp8266/include \
				$(PLATFORM_COMPONENTS)/driver/include

# Macro to make an optional library
# $1 -> The library to make
# $2 -> List of options to add to make command line
define MakeLibrary
	$(Q) $(MAKE) -C $(SMING_HOME) $(patsubst $(SMING_HOME)/%,%,$1) $2
endef

# => rBoot
RBOOT_BASE		:= $(PLATFORM_COMPONENTS)/rboot
APPCODE			+= $(RBOOT_BASE)/appcode $(RBOOT_BASE)/rboot/appcode
EXTRA_INCDIR	+= $(RBOOT_BASE)/rboot
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
	$(MAKE) -C $(PLATFORM_COMPONENTS)/rboot/rboot


# => SPIFFS
DISABLE_SPIFFS		?= 0
SPIFF_BIN_OUT		?= spiff_rom
SPIFF_BIN_OUT		:= $(FW_BASE)/$(SPIFF_BIN_OUT).bin
EXTRA_INCDIR		+= $(PLATFORM_COMPONENTS)/spiffs $(THIRD_PARTY_DIR)/spiffs/src
CFLAGS				+= -DRBOOT_SPIFFS_0=$(RBOOT_SPIFFS_0)
CFLAGS				+= -DRBOOT_SPIFFS_1=$(RBOOT_SPIFFS_1)
CUSTOM_TARGETS		+= $(SPIFF_BIN_OUT)

# => LWIP
ENABLE_CUSTOM_LWIP	?= 1
ifeq ($(ENABLE_CUSTOM_LWIP), 0)
	LIBLWIP			:= lwip
	LWIP_BASE		:= $(PLATFORM_COMPONENTS)/esp-lwip
	EXTRA_INCDIR	+= $(LWIP_BASE)/include $(LWIP_BASE)
else ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	LIBLWIP			:= lwip_open
	EXTRA_INCDIR	+= $(PLATFORM_COMPONENTS)/esp-open-lwip/esp-open-lwip/include
	ifeq ($(ENABLE_ESPCONN), 1)
		LIBLWIP		:= lwip_full
	endif
else ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	ifeq ($(ENABLE_ESPCONN), 1)
		$(error LWIP2 does not support espconn_* functions. Make sure to set ENABLE_CUSTOM_LWIP to 0 or 1.)
	endif
	LIBLWIP			:= lwip2
	LWIP2_BASE		:= $(PLATFORM_COMPONENTS)/lwip2/lwip2
	EXTRA_INCDIR	+= $(LWIP2_BASE)/glue-esp/include-esp $(LWIP2_BASE)/include
else
	EXTRA_INCDIR	+= $(PLATFORM_COMPONENTS)/esp-lwip/lwip/include
endif
LIBS				+= $(LIBLWIP)

ifneq ($(ENABLE_CUSTOM_LWIP), 0)
	LIBLWIP_DST		:= $(USER_LIBDIR)/lib$(LIBLWIP).a
	CUSTOM_TARGETS	+= $(LIBLWIP_DST)

$(LIBLWIP_DST):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_LWIP=$(ENABLE_CUSTOM_LWIP) ENABLE_ESPCONN=$(ENABLE_ESPCONN))
endif

# => GDB
APPCODE				+= $(PLATFORM_COMPONENTS)/gdbstub/appcode
EXTRA_INCDIR		+= $(PLATFORM_COMPONENTS)/gdbstub/include
ifeq ($(ENABLE_GDB), 1)
	APPCODE			+= $(PLATFORM_COMPONENTS)/gdbstub
	CUSTOM_TARGETS	+= gdb_symbols

# Copy symbols required by GDB into build directory
.PHONY: gdb_symbols
gdb_symbols: $(BUILD_BASE)/bootrom.elf

$(BUILD_BASE)/%.elf:
	$(Q) cp $(PLATFORM_COMPONENTS)/gdbstub/symbols/$(notdir $@) $@
endif

# Full GDB command line
GDB := trap '' INT; $(GDB) -x $(PLATFORM_COMPONENTS)/gdbstub/gdbcmds -b $(COM_SPEED_SERIAL) -ex "target remote $(COM_PORT)"


# => WPS
ifeq ($(ENABLE_WPS),1)
   CFLAGS	+= -DENABLE_WPS=1
   LIBS		+= wps
endif

# => MQTT
# Flags for compatability with old versions (most of them should disappear with the next major release)
ifeq ($(MQTT_NO_COMPAT),1)
	CFLAGS	+= -DMQTT_NO_COMPAT=1
endif

# => Custom heap
ENABLE_CUSTOM_HEAP ?= 0
ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN			:= mainmm
	LIBMAIN_SRC		:= $(call UserLibPath,$(LIBMAIN))
	CUSTOM_TARGETS	+= $(LIBMAIN_SRC)

$(LIBMAIN_SRC):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_HEAP=1)
else
    LIBMAIN			:= main
	LIBMAIN_SRC		:= $(SDK_LIBDIR)/lib$(LIBMAIN).a
endif

# libmain must be modified for rBoot big flash support (just one symbol gets weakened)
ifeq ($(RBOOT_BIG_FLASH),1)
	LIBMAIN			:= main2
	CFLAGS			+= -DBOOT_BIG_FLASH
	LIBMAIN_DST		:= $(BUILD_BASE)/lib$(LIBMAIN).a
	CUSTOM_TARGETS	+= $(LIBMAIN_DST)

$(LIBMAIN_DST): $(LIBMAIN_SRC)
	echo "OC $@"
	$(Q) $(OBJCOPY) -W Cache_Read_Enable_New $^ $@
endif

LIBS += $(LIBMAIN)



# SSL support using axTLS
ifeq ($(ENABLE_SSL),1)
	LIBAXTLS			:= axtls
	LIBS				+= $(LIBAXTLS)
	LIBAXTLS_DST		:= $(call UserLibPath,$(LIBAXTLS))
	AXTLS_BASE			:= $(PLATFORM_COMPONENTS)/axtls-8266/axtls-8266
	EXTRA_INCDIR		+= $(AXTLS_BASE)/.. $(AXTLS_BASE) $(AXTLS_BASE)/ssl $(AXTLS_BASE)/crypto
	AXTLS_FLAGS			:= -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1) # 
		AXTLS_FLAGS		+= -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1
	endif

	CUSTOM_TARGETS		+= $(LIBAXTLS_DST) include/ssl/private_key.h
	CFLAGS				+= $(AXTLS_FLAGS)

$(LIBAXTLS_DST):
	$(call MakeLibrary,$@,ENABLE_SSL=1)

include/ssl/private_key.h:
	$(vecho) "Generating unique certificate and key. This may take some time"
	$(Q) mkdir -p $(CURRENT_DIR)/include/ssl/
	AXDIR=$(CURRENT_DIR)/include/ssl/ $(PLATFORM_COMPONENTS)/axtls-8266/axtls-8266/tools/make_certs.sh
endif


# => PWM
ENABLE_CUSTOM_PWM ?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	LIBPWM			:= pwm_open
	LIBPWM_DST		:= $(call UserLibPath,$(LIBPWM))
	CUSTOM_TARGETS	+= $(LIBPWM_DST)

$(LIBPWM_DST):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_PWM=1)
else
	LIBPWM := pwm
endif
LIBS += $(LIBPWM)


LIBS := microc microgcc hal phy pp net80211 mqttc wpa $(LIBSMING) crypto smartconfig $(EXTRA_LIBS) $(LIBS)

LIBS := $(addprefix -l,$(LIBS))

# linker flags used to generate the main object file
LDFLAGS	= -nostdlib -u call_user_start -u Cache_Read_Enable_New -u spiffs_get_storage_config -u custom_crash_callback \
			-Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map -Wl,-wrap,system_restart_local 

include $(PLATFORM_BASE)/flash.mk

TARGET_OUT_0 := $(BUILD_BASE)/$(TARGET)_0.out
TARGET_OUT_1 := $(BUILD_BASE)/$(TARGET)_1.out

#############
#
# Target definitions
#
#############

include $(SMING_HOME)/modules.mk

# Add APPCODE objects and targets
$(call ScanModules,$(APPCODE))

app: $(CUSTOM_TARGETS) $(RBOOT_ROM_0) $(RBOOT_ROM_1) $(FW_FILE_1) $(FW_FILE_2)

spiff_update: spiff_clean $(SPIFF_BIN_OUT)

# => Firmware images
$(RBOOT_ROM_0): $(TARGET_OUT_0)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

$(RBOOT_ROM_1): $(TARGET_OUT_1)
	$(vecho) "E2 $@"
	$(Q) $(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $< $@ $(RBOOT_E2_SECTS)

$(TARGET_OUT_0): $(APP_AR)
	$(vecho) "LD $@"
	$(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(PLATFORM_BASE)/Compiler/ld \
		-T$(RBOOT_LD_0) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@

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


$(TARGET_OUT_1): $(APP_AR)
	$(vecho) "LD $@"
	$(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(PLATFORM_BASE)/Compiler/ld \
		-T$(RBOOT_LD_1) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@

# recreate it from 0, since you get into problems with same filenames
$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) test ! -f $@ || rm $@
	$(AR) rcsP $@ $^

.PHONY: libsming
libsming: $(USER_LIBDIR)/lib$(LIBSMING).a
$(USER_LIBDIR)/lib$(LIBSMING).a:
	$(vecho) "(Re)compiling Sming. Enabled features: $(SMING_FEATURES). This may take some time"
	$(Q) $(MAKE) -C $(SMING_HOME) clean V=$(V) ENABLE_SSL=$(ENABLE_SSL)
	$(Q) $(MAKE) -C $(SMING_HOME) V=$(V) ENABLE_SSL=$(ENABLE_SSL)


checkdirs: | $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR) $(FW_BASE):
	$(Q) mkdir -p $@

spiff_clean: 
	$(vecho) "Cleaning $(SPIFF_BIN_OUT)"
	$(Q) rm -rf $(SPIFF_BIN_OUT)

$(SPIFF_BIN_OUT):
ifeq ($(DISABLE_SPIFFS), 1)
	$(vecho) "(!) Spiffs support disabled. Remove 'DISABLE_SPIFFS' make argument to enable spiffs."
else
# Generating spiffs_bin
	$(vecho) "Checking for spiffs files"
	$(Q)	if [ -d "$(SPIFF_FILES)" ]; then \
				echo "$(SPIFF_FILES) directory exists. Creating $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) $(SPIFF_FILES) $(SPIFF_BIN_OUT); \
			else \
				echo "No files found in ./$(SPIFF_FILES)."; \
				echo "Creating empty $(SPIFF_BIN_OUT)"; \
				$(SPIFFY) $(SPIFF_SIZE) dummy.dir $(SPIFF_BIN_OUT); \
			fi
endif


# Flashing

flashboot: libsming $(RBOOT_BIN)
	$(WRITE_FLASH) 0x00000 $(RBOOT_BIN)

flashconfig: kill_term
	$(vecho) "Deleting rBoot config sector"
	$(WRITE_FLASH) 0x01000 $(SDK_BASE)/bin/blank.bin 

flashapp: all kill_term
	$(WRITE_FLASH) $(ROM_0_ADDR) $(RBOOT_ROM_0)

flashfs: libsming $(SPIFF_BIN_OUT)
ifeq ($(DISABLE_SPIFFS), 1)
	$(vecho) "SPIFFS are not enabled!"
else
	$(WRITE_FLASH) $(RBOOT_SPIFFS_0) $(SPIFF_BIN_OUT)
endif

# flashes rboot and first rom
flash: all kill_term
	FLASH_CHUNKS := 0x00000 $(RBOOT_BIN)
	FLASH_CHUNKS += $(ROM_0_ADDR) $(RBOOT_ROM_0)
	ifneq ($(DISABLE_SPIFFS), 1)
		FLASH_CHUNKS += $(RBOOT_SPIFFS_0) $(SPIFF_BIN_OUT)
	endif
	$(WRITE_FLASH) $(FLASH_CHUNKS)
	ifeq ($(ENABLE_GDB), 1)
		$(GDB)
	else
		$(TERMINAL)
	endif

otaserver: all
	$(vecho) "Starting OTA server for TESTING"
	$(Q) cd $(FW_BASE) && python -m SimpleHTTPServer $(SERVER_OTA_PORT)

kill_term:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)

terminal: kill_term
	$(TERMINAL)

gdb: kill_term
	$(GDB)

# Wipe flash
flashinit:
	FLASH_INIT_CHUNKS := $(INIT_BIN_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin
	FLASH_INIT_CHUNKS += $(BLANK_BIN_ADDR) $(SDK_BASE)/bin/blank.bin
	ifneq ($(DISABLE_SPIFFS), 1)
		FLASH_INIT_CHUNKS += $(RBOOT_SPIFFS_0) $(PLATFORM_BASE)/Compiler/data/blankfs.bin
	endif
	$(vecho) "Flash init data default and blank data."
	$(vecho) "DISABLE_SPIFFS = $(DISABLE_SPIFFS)"
	$(ERASE_FLASH)
	$(WRITE_FLASH) $(FLASH_INIT_CHUNKS)


rebuild: clean all

# Remove build artifacts
clean:
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

