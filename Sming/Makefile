#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

## Defaults for Sming Core

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

# Com port speed
COM_SPEED = 115200


ifeq ($(OS),Windows_NT)
  # Windows detected
  UNAME := Windows
  SMING_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(SMING_HOME))))
  include $(SMING_HOME)/Makefile-windows.mk  
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME),Darwin)
      UNAME := MacOS
      include $(SMING_HOME)/Makefile-macos.mk      
  endif
  ifeq ($(UNAME),Linux)
      #Linux
      UNAME := Linux
      include $(SMING_HOME)/Makefile-linux.mk     
  endif
endif

export COMPILE := gcc
export PATH := "$(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)"
XTENSA_TOOLS_ROOT ?= $(ESP_HOME)/xtensa-lx106-elf/bin

# Sming Framework Path
SMF = $(SMING_HOME)

SPIFF_FILES = files

BUILD_BASE	= out/build
FW_BASE		= out/firmware

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= $(SMING_HOME)/system $(SMING_HOME)/system/helpers $(SMING_HOME)/Wiring $(SMING_HOME)/SmingCore $(SMING_HOME)/SmingCore/Network $(SMING_HOME)/SmingCore/Platform $(SMING_HOME)/Services/SpifFS $(SMING_HOME)/Services/ArduinoJson $(SMING_HOME)/Services/WebHelpers $(SMING_HOME)/Services/DateTime $(SMING_HOME)/appinit app $(SMING_HOME)/Libraries/DHT $(SMING_HOME)/Libraries/LiquidCrystal $(SMING_HOME)/Libraries/Bounce $(SMING_HOME)/Libraries/Adafruit_GFX $(SMING_HOME)/Libraries/TFT_ILI9163C $(SMING_HOME)/Libraries/BMP180 $(SMING_HOME)/Libraries/BH1750FVI $(SMING_HOME)/Libraries/Adafruit_SSD1306 $(SMING_HOME)/Libraries/OneWire $(SMING_HOME)/Libraries/I2Cdev $(SMING_HOME)/Libraries/HMC5883L
EXTRA_INCDIR    = include $(SMING_HOME)/ $(SMING_HOME)/system/include $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore $(SDK_BASE)/../include

# libraries used in this project, mainly provided by the SDK
USER_LIBDIR = $(SMING_HOME)/compiler/lib
LIBS		= microc gcc hal phy pp net80211 openlwip wpa main

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -D__ets__ -DICACHE_FLASH -D ARDUINO=106
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

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

# test:
# 	#$(vecho) $(OBJ) $(APP_AR)
# 	$(vecho) $(BUILD_DIR)

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
$1/%.o: %.cpp
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(vecho) "Run objcopy, please wait..."
	$(Q) $(OBJCOPY) --only-section .text -O binary $@ eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $@ eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $@ eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $@ eagle.app.v6.irom0text.bin
	$(vecho) "objcopy done"
	$(vecho) "Run gen_appbin"
ifeq ($(app), 0)
	$(Q) $(GEN_APPBIN) $@ 0 $(mode) $(freqdiv) $(size)
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
	$(vecho) "Adding init with 0x10000 offset"
#	$(Q) srec_cat -output $(FW_BASE)/eagle.irom0text.fs.bin -binary $(FW_BASE)/eagle.flash.bin -binary -fill 0xff 0x00000 0x10000 $(FW_BASE)/eagle.irom0text.bin -binary -offset 0x10000 > srec_cat.log
	
	$(Q) cat $(FW_BASE)/eagle.irom0text.bin $(SMING_HOME)/compiler/data/blankfs.bin > $(FW_BASE)/eagle.irom0text.fs.bin
	$(Q) rm eagle.app.v6.*
	
#	# Getting flash file size to get correct spiffs offset
#	# Way 1
#	$(vecho) "flash size: $$(printf '%X\n' $$($(GET_FILESIZE) $(FW_BASE)/eagle.flash.bin))"

#	# Way 2
#	0x$(echo "obase=16; $(($(stat --format="%s" $BUILD_DIR/bin/${x00000})))" | bc)
#	$(vecho) "main fw offset: $$(echo "obase=16; $(($(GET_FILESIZE) $(FW_BASE)/eagle.flash.bin))" | bc)	
	
#	# Generating spiffs_bin
#	$(vecho) "Checking for spiffs files"
#	$(Q) if [ -d "$(SPIFF_FILES)" ]; then \
#        echo "Found $(SPIFF_FILES) directory. Creating spiff_rom.bin"; \
#        spiffy; \
#        mv spiff_rom.bin $(FW_BASE)/spiff_rom.bin; \
#	else \
#         echo "No files found in $(SPIFF_FILES)"; \
#	fi
		
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->0x10000"	
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

flashonefile: all
	$(OBJCOPY) --only-section .text -O binary $(TARGET_OUT) eagle.app.v6.text.bin
	$(OBJCOPY) --only-section .data -O binary $(TARGET_OUT) eagle.app.v6.data.bin
	$(OBJCOPY) --only-section .rodata -O binary $(TARGET_OUT) eagle.app.v6.rodata.bin
	$(OBJCOPY) --only-section .irom0.text -O binary $(TARGET_OUT) eagle.app.v6.irom0text.bin
	$(GEN_APPBIN) $(TARGET_OUT) v6
	$(GEN_FLASHBIN) eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin 0x10000
	rm -f eagle.app.v6.data.bin
	rm -f eagle.app.v6.flash.bin
	rm -f eagle.app.v6.irom0text.bin
	rm -f eagle.app.v6.rodata.bin
	rm -f eagle.app.v6.text.bin
	rm -f eagle.app.sym
	mv eagle.app.flash.bin $(FW_BASE)/
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.app.flash.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.app.flash.bin-------->0x00000"
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.app.flash.bin

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
	$(KILL_TERM)	
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x10000 $(FW_BASE)/eagle.irom0text.bin
else
ifeq ($(boot), none)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED) write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x10000 $(FW_BASE)/eagle.irom0text.bin
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