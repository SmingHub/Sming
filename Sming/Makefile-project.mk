#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

# Com port default speed
COM_SPEED ?= 230400

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

SPIFF_FILES = files

BUILD_BASE	= out/build
FW_BASE		= out/firmware

SPIFF_START_OFFSET = $(shell printf '0x%X\n' $$(( ($$($(GET_FILESIZE) $(FW_BASE)/0x09000.bin) + 16384 + 36864) & (0xFFFFC000) )) )

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES 	?= app  # if not initialized by user 
MODULES		+= $(SMING_HOME)/appinit
EXTRA_INCDIR    ?= include $(SMING_HOME)/include $(SMING_HOME)/ $(SMING_HOME)/system/include $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore $(SDK_BASE)/../include

# libraries used in this project, mainly provided by the SDK
USER_LIBDIR = $(SMING_HOME)/compiler/lib/
LIBS		= microc microgcc hal phy pp net80211 lwip wpa main sming $(EXTRA_LIBS)

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections -D__ets__ -DICACHE_FLASH -DARDUINO=106
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

# we will use global WiFi settings from Eclipse Environment Variables, if possible
WIFI_SSID ?= ""
WIFI_PWD ?= ""
ifneq ($(WIFI_SSID), "")
	CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifneq ($(WIFI_PWD), "")
	CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\"
endif

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -u call_user_start -Wl,-static -Wl,--gc-sections

# linker script used for the above linkier step
LD_PATH     = $(SMING_HOME)/compiler/ld/
LD_SCRIPT	= $(LD_PATH)eagle.app.v6.cpp.ld

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
else
    ifeq ($(SPI_SIZE), 1M)
	flashimageoptions += -fs 8m
    else
        ifeq ($(SPI_SIZE), 2M)
		flashimageoptions += -fs 16m
        else
            ifeq ($(SPI_SIZE), 4M)
			flashimageoptions += -fs 32m
            else
			flashimageoptions += -fs 4m
            endif
        endif
    endif
endif

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
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

SPIFF_BIN_OUT := $(FW_BASE)/spiff_rom.bin
LD_SCRIPT	:= $(addprefix -T,$(LD_SCRIPT))

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

all: checkdirs $(TARGET_OUT) $(SPIFF_BIN_OUT) $(FW_FILE_1) $(FW_FILE_2)

spiff_update: spiff_clean $(SPIFF_BIN_OUT)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"	
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	
	$(vecho) "------------------------------------------------------------------------------"
ifeq ($(UNAME),Windows)
	$(vecho) "Memory layout info:"
	$(Q) $(SDK_TOOLS)/memanalyzer.exe $(OBJDUMP).exe $@
else
	$(vecho) "Section info:"
	$(Q) $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $@
endif
	$(vecho) "------------------------------------------------------------------------------"
	
	$(vecho) "# Generating image..."
	$(ESPTOOL) elf2image $@ $(flashimageoptions) -o $(FW_BASE)/
	$(vecho) "Generate firmware images successully in folder $(FW_BASE)."
	$(vecho) "Done"

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
	# Generating spiffs_bin
	$(vecho) "Checking for spiffs files"
	$(Q) if [ -d "$(SPIFF_FILES)" ]; then \
    	echo "$(SPIFF_FILES) directory exists. Creating spiff_rom.bin"; \
    	spiffy; \
    	mv spiff_rom.bin $(FW_BASE)/spiff_rom.bin; \
	else \
    	echo "No files found in ./$(SPIFF_FILES)."; \
    	echo "Creating empty spiff_rom.bin ($$($(GET_FILESIZE) $(SMING_HOME)/compiler/data/blankfs.bin) bytes)"; \
    cp $(SMING_HOME)/compiler/data/blankfs.bin $(FW_BASE)/spiff_rom.bin; \
	fi
	$(vecho) "spiff_rom.bin---------->$(SPIFF_START_OFFSET)"

flash: all
	-$(KILL_TERM)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash $(flashimageoptions) 0x00000 $(FW_BASE)/0x00000.bin 0x09000 $(FW_BASE)/0x09000.bin $(SPIFF_START_OFFSET) $(FW_BASE)/spiff_rom.bin

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash $(flashimageoptions) 0x79000 $(SDK_BASE)/bin/clear_eep.bin 0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin 0x7e000 $(SDK_BASE)/bin/blank.bin 0x4B000 $(SMING_HOME)/compiler/data/blankfs.bin

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
