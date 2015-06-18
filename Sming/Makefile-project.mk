#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

## Defaults

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

# Com port default speed
COM_SPEED ?= 230400


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

# Sming Framework Path
SMF = $(SMING_HOME)

SPIFF_FILES = files

BUILD_BASE	= out/build
FW_BASE		= out/firmware

#$(eval fw_start_offset := $(shell printf '0x%X\n' $$( $(GET_FILESIZE) $(FW_BASE)/eagle.flash.bin )))
#$(eval spiff_start_offset := $(shell printf '0x%X\n' $$(( ($$($(GET_FILESIZE) $(FW_BASE)/eagle.irom0text.bin) + 16384 + 36864) & (0xFFFFC000) )) ))
FW_START_OFFSET = $(shell printf '0x%X\n' $$( $(GET_FILESIZE) $(FW_BASE)/eagle.flash.bin ))
SPIFF_START_OFFSET = $(shell printf '0x%X\n' $$(( ($$($(GET_FILESIZE) $(FW_BASE)/eagle.irom0text.bin) + 16384 + 36864) & (0xFFFFC000) )) )

	
# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES		+= $(SMING_HOME)/appinit
EXTRA_INCDIR    ?= include $(SMING_HOME)/include $(SMING_HOME)/ $(SMING_HOME)/system/include $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore $(SDK_BASE)/../include

# libraries used in this project, mainly provided by the SDK
USER_LIBDIR = $(SMING_HOME)/compiler/lib/
LIBS		= microc microgcc hal phy pp net80211 lwip wpa main sming $(EXTRA_LIBS)

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections -D__ets__ -DICACHE_FLASH -DARDUINO=106
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

# trying to use global WiFi settings from Eclipse Environment Variables
WIFI_SSID ?= ""
ifneq ($(WIFI_SSID), "")
	CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
WIFI_PWD ?= ""
ifneq ($(WIFI_PWD), "")
	CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\"
endif

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -u call_user_start -Wl,-static -Wl,--gc-sections

# linker script used for the above linkier step
LD_PATH     = $(SMING_HOME)/compiler/ld/
LD_SCRIPT	= $(LD_PATH)eagle.app.v6.cpp.ld

# BOOT = none
# BOOT = old - boot_v1.1
# BOOT = new - boot_v1.2+
BOOT?=none
# APP = 0 - eagle.flash.bin + eagle.irom0text.bin
# APP = 1 - user1.bin
# APP = 2 - user2.bin
APP?=0
# SPI_SPEED = 20MHz, 26.7MHz, 40MHz, 80MHz
SPI_SPEED?=40
# SPI_MODE: QIO, QOUT, DIO, DOUT
SPI_MODE?=QIO
# SPI_SIZE: 256KB, 512KB, 1024KB, 2048KB, 4096KB
SPI_SIZE?=512

ifeq ($(BOOT), new)
    boot = new
else
    ifeq ($(BOOT), old)
        boot = old
    else
        boot = none
    endif
endif

ifeq ($(APP), 1)
    app = 1
else
    ifeq ($(APP), 2)
        app = 2
    else
        app = 0
    endif
endif

ifeq ($(SPI_SPEED), 26.7)
    freqdiv = 1
else
    ifeq ($(SPI_SPEED), 20)
        freqdiv = 2
    else
        ifeq ($(SPI_SPEED), 80)
            freqdiv = 15
        else
            freqdiv = 0
        endif
    endif
endif


ifeq ($(SPI_MODE), QOUT)
    mode = 1
else
    ifeq ($(SPI_MODE), DIO)
        mode = 2
    else
        ifeq ($(SPI_MODE), DOUT)
            mode = 3
        else
            mode = 0
        endif
    endif
endif

# flash larger than 1024KB only use 1024KB to storage user1.bin and user2.bin
ifeq ($(SPI_SIZE), 256)
    size = 1
    flash = 256
else
    ifeq ($(SPI_SIZE), 1024)
        size = 2
        flash = 1024
    else
        ifeq ($(SPI_SIZE), 2048)
            size = 3
            flash = 1024
        else
            ifeq ($(SPI_SIZE), 4096)
                size = 4
                flash = 1024
            else
                size = 0
                flash = 512
            endif
        endif
    endif
endif

ifeq ($(flash), 512)
  ifeq ($(app), 1)
    addr = 0x01000
  else
    ifeq ($(app), 2)
      addr = 0x41000
    endif
  endif
else
  ifeq ($(flash), 1024)
    ifeq ($(app), 1)
      addr = 0x01000
    else
      ifeq ($(app), 2)
        addr = 0x81000
      endif
    endif
  endif
endif

ifneq ($(boot), none)
ifneq ($(app),0)
	LD_SCRIPT = $(LD_PATH)eagle.app.v6.$(boot).$(flash).app$(app).cpp.ld
	BIN_NAME = user$(app).$(flash).$(boot)
endif
else
    app = 0
endif

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

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
#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
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
	
	$(vecho) "Running objcopy, please wait..."	
	$(Q) $(OBJCOPY) --only-section .text -O binary $@ eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $@ eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $@ eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $@ eagle.app.v6.irom0text.bin
	$(vecho) "Objcopy done"
	$(vecho) "Run gen_appbin"
ifeq ($(app), 0)
	$(Q) $(GEN_APPBIN) $@ 0 $(mode) $(freqdiv) $(size)
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
#	$(vecho) "Adding init with 0x9000 offset"
#	$(Q) srec_cat -output $(FW_BASE)/eagle.irom0text.fs.bin -binary $(FW_BASE)/eagle.flash.bin -binary -fill 0xff 0x00000 0x9000 $(FW_BASE)/eagle.irom0text.bin -binary -offset 0x9000 > srec_cat.log
	
#	$(Q) cat $(FW_BASE)/eagle.irom0text.bin $(SMING_HOME)/compiler/data/blankfs.bin > $(FW_BASE)/eagle.irom0text.fs.bin

	$(Q) rm eagle.app.v6.*
#	$(vecho) "Firmware offset: $(FW_START_OFFSET)"
#	$(vecho) "Spiffs offset: $(SPIFF_START_OFFSET)"

	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->0x9000"
else
    ifeq ($(boot), new)
		$(Q) $(GEN_APPBIN) $@ 2 $(mode) $(freqdiv) $(size)
		$(vecho) "Support boot_v1.2 and +"
    else
		$(Q) $(GEN_APPBIN) $@ 1 $(mode) $(freqdiv) $(size)
		$(vecho) "Support boot_v1.1 and +"
    endif
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/upgrade/$(BIN_NAME).bin
	$(Q) rm eagle.app.v6.*
	$(vecho) "Generate $(BIN_NAME).bin successully in folder $(FW_BASE)/upgrade."
	$(vecho) "boot_v1.x.bin------->0x00000"
	$(vecho) "$(BIN_NAME).bin--->$(addr)"
endif
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@
	$(Q) mkdir -p $@/upgrade

# flashonefile: all
# 	$(OBJCOPY) --only-section .text -O binary $(TARGET_OUT) eagle.app.v6.text.bin
# 	$(OBJCOPY) --only-section .data -O binary $(TARGET_OUT) eagle.app.v6.data.bin
# 	$(OBJCOPY) --only-section .rodata -O binary $(TARGET_OUT) eagle.app.v6.rodata.bin
# 	$(OBJCOPY) --only-section .irom0.text -O binary $(TARGET_OUT) eagle.app.v6.irom0text.bin
# 	$(GEN_APPBIN) $(TARGET_OUT) v6
# 	$(GEN_FLASHBIN) eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin 0x9000
# 	rm -f eagle.app.v6.data.bin
# 	rm -f eagle.app.v6.flash.bin
# 	rm -f eagle.app.v6.irom0text.bin
# 	rm -f eagle.app.v6.rodata.bin
# 	rm -f eagle.app.v6.text.bin
# 	rm -f eagle.app.sym
# 	mv eagle.app.flash.bin $(FW_BASE)/
# 	$(vecho) "No boot needed."
# 	$(vecho) "Generate eagle.app.flash.bin successully in folder $(FW_BASE)."
# 	$(vecho) "eagle.app.flash.bin-------->0x00000"
# 	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.app.flash.bin



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

flashboot: all flashinit
ifeq ($(boot), new)
	$(vecho) "Flash boot_v1.2 and +"
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(SDK_BASE)/bin/boot_v1.2.bin
endif
ifeq ($(boot), old)
	$(vecho) "Flash boot_v1.1 and +"
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(SDK_BASE)/bin/boot_v1.1.bin
endif
ifeq ($(boot), none)
	$(vecho) "No boot needed."
endif

flash: all
ifeq ($(app), 0)	
#	$(Q) $(KILL_TERM)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x9000 $(FW_BASE)/eagle.irom0text.bin $(SPIFF_START_OFFSET) $(FW_BASE)/spiff_rom.bin
else
ifeq ($(boot), none)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x9000 $(FW_BASE)/eagle.irom0text.bin
else
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash $(addr) $(FW_BASE)/upgrade/$(BIN_NAME).bin
endif
endif

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(COM_PORT) write_flash 0x4B000 $(SMF)/compiler/data/blankfs.bin 0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin 0x7e000 $(SDK_BASE)/bin/blank.bin

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
