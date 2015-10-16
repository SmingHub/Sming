#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

# rBoot options, overwrite them in the projects Makefile-user.mk
RBOOT_BIG_FLASH  ?= 1
RBOOT_TWO_ROMS   ?= 0
RBOOT_ROM_0      ?= rom0
RBOOT_ROM_1      ?= rom1
RBOOT_SPIFFS_0   ?= 0x100000
RBOOT_SPIFFS_1   ?= 0x300000
RBOOT_LD_0 ?= rom0.ld
RBOOT_LD_1 ?= rom1.ld
# esptool2 path
ESPTOOL2 ?= esptool2
# path to spiffy
SPIFFY ?= $(SMING_HOME)/spiffy/spiffy
# filenames and options for generating rBoot rom images with esptool2
RBOOT_E2_SECTS     ?= .text .data .rodata
RBOOT_E2_USER_ARGS ?= -quiet -bin -boot2

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

## Flash parameters
# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED ?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE ?= qio
# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE ?= 512K

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
# SMING_HOME = /opt/esp-open-sdk

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

# MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

ifeq ($(OS),Windows_NT)
  # Windows detected
  UNAME := Windows
  
  # Default SMING_HOME. Can be overriden.
  SMING_HOME ?= c:\tools\Sming\Sming

  # Default ESP_HOME. Can be overriden.
  ESP_HOME ?= c:\Espressif

  # Making proper path adjustments - replace back slashes, remove colon and add forward slash.
  SMING_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(SMING_HOME))))
  ESP_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(ESP_HOME))))
  include $(SMING_HOME)/Makefile-windows.mk  
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME),Darwin)
      # MacOS Detected
      UNAME := MacOS

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /opt/sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /opt/esp-open-sdk

      include $(SMING_HOME)/Makefile-macos.mk      
  endif
  ifeq ($(UNAME),Linux)
      # Linux Detected
      UNAME := Linux

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /opt/sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /opt/esp-open-sdk
      include $(SMING_HOME)/Makefile-linux.mk     
  endif
  ifeq ($(UNAME),FreeBSD)
      # Freebsd Detected
      UNAME := FreeBSD

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /usr/local/esp8266/Sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /usr/local/esp8266/esp-open-sdk
      include $(SMING_HOME)/Makefile-bsd.mk     
  endif
endif

export COMPILE := gcc
export PATH := $(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)
XTENSA_TOOLS_ROOT := $(ESP_HOME)/xtensa-lx106-elf/bin

SPIFF_FILES ?= files

BUILD_BASE	= out/build
FW_BASE		= out/firmware

RBOOT_ROM_0  := $(addprefix $(FW_BASE)/,$(RBOOT_ROM_0).bin)
RBOOT_ROM_1  := $(addprefix $(FW_BASE)/,$(RBOOT_ROM_1).bin)

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES 	?= app  # if not initialized by user 
MODULES		+= $(SMING_HOME)/rboot/appcode
EXTRA_INCDIR    ?= include $(SMING_HOME)/include $(SMING_HOME)/ $(SMING_HOME)/system/include $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore $(SDK_BASE)/../include $(SMING_HOME)/rboot $(SMING_HOME)/rboot/appcode

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections -D__ets__ -DICACHE_FLASH -DARDUINO=106
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

# libmain must be modified for rBoot big flash support (just one symbol gets weakened)
ifeq ($(RBOOT_BIG_FLASH),1)
	LIBMAIN = main2
	LIBMAIN_SRC = $(addprefix $(SDK_LIBDIR)/,libmain.a)
	LIBMAIN_DST = $(addprefix $(BUILD_BASE)/,libmain2.a)
	CFLAGS += -DBOOT_BIG_FLASH
else
	LIBMAIN = main
	LIBMAIN_DST = $()
endif
# libraries used in this project, mainly provided by the SDK
USER_LIBDIR = $(SMING_HOME)/compiler/lib/
LIBS		= microc microgcc hal phy pp net80211 lwip wpa $(LIBMAIN) sming $(EXTRA_LIBS)

# we will use global WiFi settings from Eclipse Environment Variables, if possible
WIFI_SSID ?= ""
WIFI_PWD ?= ""
ifneq ($(WIFI_SSID), "")
	CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifneq ($(WIFI_PWD), "")
	CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\"
endif
ifeq ($(DISABLE_SPIFFS), 1)
	CFLAGS += -DDISABLE_SPIFFS=1
endif

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -u call_user_start -u Cache_Read_Enable_New -Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map

ifeq ($(SPI_SPEED), 26)
	flashimageoptions = -ff 26m
else
    ifeq ($(SPI_SPEED), 20)
        flashimageoptions = -ff 20m
    else
        ifeq ($(SPI_SPEED), 80)
		flashimageoptions = -ff 80m
        else
		flashimageoptions = -ff 40m
        endif
    endif
endif

ifeq ($(SPI_MODE), qout)
	flashimageoptions += -fm qout
else
    ifeq ($(SPI_MODE), dio)
	flashimageoptions += -fm dio
    else
        ifeq ($(SPI_MODE), dout)
		flashimageoptions += -fm dout
        else
		flashimageoptions += -fm qio
        endif
    endif
endif

# flash larger than 1024KB only use 1024KB to storage user1.bin and user2.bin
ifeq ($(SPI_SIZE), 256K)
	flashimageoptions += -fs 2m
	SPIFF_SIZE ?= 131072  #128K
else
    ifeq ($(SPI_SIZE), 1M)
	flashimageoptions += -fs 8m
	SPIFF_SIZE ?= 524288  #512K
    else
        ifeq ($(SPI_SIZE), 2M)
		flashimageoptions += -fs 16m
		SPIFF_SIZE ?= 524288  #512K
        else
            ifeq ($(SPI_SIZE), 4M)
			flashimageoptions += -fs 32m
			SPIFF_SIZE ?= 524288  #512K
            else
			flashimageoptions += -fs 4m
			SPIFF_SIZE ?= 262144  #256K
            endif
        endif
    endif
endif
CFLAGS += -DSPIFF_SIZE=$(SPIFF_SIZE)
CFLAGS += -DRBOOT_SPIFFS_0=$(RBOOT_SPIFFS_0)
CFLAGS += -DRBOOT_SPIFFS_1=$(RBOOT_SPIFFS_1)

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CXX		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
C_OBJ		:= $(patsubst %.c,%.o,$(SRC))
CXX_OBJ		:= $(patsubst %.cpp,%.o,$(C_OBJ))
OBJ		:= $(patsubst %.o,$(BUILD_BASE)/%.o,$(CXX_OBJ))

LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT_0 := $(addprefix $(BUILD_BASE)/,$(TARGET)_0.out)
TARGET_OUT_1 := $(addprefix $(BUILD_BASE)/,$(TARGET)_1.out)

SPIFF_BIN_OUT ?= spiff_rom
SPIFF_BIN_OUT := $(FW_BASE)/$(SPIFF_BIN_OUT).bin
RBOOT_LD_0	:= $(addprefix -T,$(RBOOT_LD_0))
RBOOT_LD_1	:= $(addprefix -T,$(RBOOT_LD_1))

# extra flags
CFLAGS += -DRBOOT_INTEGRATION

RBOOT_BIN := $(FW_BASE)/rboot.bin
RBOOT_BUILD_BASE := $(abspath $(BUILD_BASE))
RBOOT_FW_BASE := $(abspath $(FW_BASE))
# these are exported for use by the rBoot Makefile
export RBOOT_BIG_FLASH
export RBOOT_BUILD_BASE
export RBOOT_FW_BASE
export SPI_SIZE
export ESPTOOL2
export SDK_BASE

# multiple roms per 1mb block?
ifeq ($(RBOOT_TWO_ROMS),1)
	# set a define so ota code can choose correct rom
	CFLAGS += -DRBOOT_TWO_ROMS
else
	# eliminate the second rBoot target
	RBOOT_ROM_1 := $()
endif

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@	
$1/%.o: %.cpp
	$(vecho) "C+ $$<" 
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS) -c $$< -o $$@
endef

.PHONY: all checkdirs spiff_update spiff_clean clean

all: checkdirs $(LIBMAIN_DST) $(RBOOT_BIN) $(RBOOT_ROM_0) $(RBOOT_ROM_1) $(SPIFF_BIN_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(RBOOT_BIN):
	$(MAKE) -C $(SMING_HOME)/rboot

$(LIBMAIN_DST): $(LIBMAIN_SRC)
	@echo "OC $@"
	@$(OBJCOPY) -W Cache_Read_Enable_New $^ $@

spiff_update: spiff_clean $(SPIFF_BIN_OUT)

$(RBOOT_ROM_0): $(TARGET_OUT_0)
	@echo "E2 $@"
	@$(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $(TARGET_OUT_0) $@ $(RBOOT_E2_SECTS)

$(RBOOT_ROM_1): $(TARGET_OUT_1)
	@echo "E2 $@"
	@$(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $(TARGET_OUT_1) $@ $(RBOOT_E2_SECTS)

$(TARGET_OUT_0): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) $(RBOOT_LD_0) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@

$(TARGET_OUT_1): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) $(RBOOT_LD_1) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
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
	$(Q) if [ -d "$(SPIFF_FILES)" ]; then \
		echo "$(SPIFF_FILES) directory exists. Creating $(SPIFF_BIN_OUT)"; \
		$(SPIFFY) $(SPIFF_SIZE) $(SPIFF_FILES) $(SPIFF_BIN_OUT); \
	else \
		echo "No files found in ./$(SPIFF_FILES)."; \
		echo "Creating empty $(SPIFF_BIN_OUT) ($$($(GET_FILESIZE) $(SMING_HOME)/compiler/data/blankfs.bin) bytes)"; \
		cp $(SMING_HOME)/compiler/data/blankfs.bin $(SPIFF_BIN_OUT); \
	fi
endif

flash: all
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)
ifeq ($(DISABLE_SPIFFS), 1)
# flashes rboot and first rom
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x00000 $(RBOOT_BIN) 0x02000 $(RBOOT_ROM_0)
else
# flashes rboot, first rom and spiffs
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x00000 $(RBOOT_BIN) 0x02000 $(RBOOT_ROM_0) $(RBOOT_SPIFFS_0) $(SPIFF_BIN_OUT)
endif
	$(TERMINAL)

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin 0x7e000 $(SDK_BASE)/bin/blank.bin 0x4B000 $(SMING_HOME)/compiler/data/blankfs.bin

rebuild: clean all

clean:
#remove build artifacts
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
