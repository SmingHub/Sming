#############################################################
#
# Root Level Makefile
#
# (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

# Sming Framework Path
SMF = ../Sming/

BUILD_BASE	= out/build
FW_BASE		= out/firmware

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK_1_00

# esptool path and port
SDK_TOOLS	?= c:/Espressif/utils
ESPTOOL		?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM21

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= $(SMF)system $(SMF)system/helpers $(SMF)Wiring $(SMF)SmingCore $(SMF)SmingCore/Network $(SMF)SmingCore/Platform $(SMF)Services/SpifFS $(SMF)Services/ArduinoJson $(SMF)Services/WebHelpers $(SMF)Services/DateTime $(SMF)appinit app $(SMF)Libraries/DHT $(SMF)Libraries/LiquidCrystal $(SMF)Libraries/Bounce $(SMF)Libraries/Adafruit_GFX $(SMF)Libraries/TFT_ILI9163C $(SMF)Libraries/BMP180 $(SMF)Libraries/BH1750FVI $(SMF)Libraries/Adafruit_SSD1306 $(SMF)Libraries/OneWire $(SMF)Libraries/I2Cdev $(SMF)Libraries/HMC5883L
EXTRA_INCDIR    = include $(SMF) $(SMF)system/include $(SMF)Wiring $(SMF)Libraries $(SMF)SmingCore $(SDK_BASE)/../include

# libraries used in this project, mainly provided by the SDK
USER_LIBDIR = $(SMF)compiler/lib
LIBS		= c m gcc hal phy pp net80211 openlwip wpa main

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions \
				-nostdlib -mlongcalls -mtext-section-literals -D__ets__ -DICACHE_FLASH -D ARDUINO=106
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_PATH     = $(SMF)compiler/ld/
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
	$(vecho) "Run gen_appbin.exe"
ifeq ($(app), 0)
	$(Q) $(SDK_TOOLS)/gen_appbin.exe $@ 0 $(mode) $(freqdiv) $(size)
	$(Q) mv eagle.app.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
	$(Q) rm eagle.app.v6.*
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->0x10000"
else
    ifeq ($(boot), new)
		$(Q) $(SDK_TOOLS)/gen_appbin.exe $@ 2 $(mode) $(freqdiv) $(size)
		$(vecho) "Support boot_v1.2 and +"
    else
		$(Q) $(SDK_TOOLS)/gen_appbin.exe $@ 1 $(mode) $(freqdiv) $(size)
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
	$(SDK_TOOLS)/gen_appbin_old.exe $(TARGET_OUT) v6
	$(SDK_TOOLS)/gen_flashbin.exe eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin 0x10000
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
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash 0x00000 $(FW_BASE)/eagle.app.flash.bin

flashboot: all flashinit
ifeq ($(boot), new)
	$(vecho) "Flash boot_v1.2 and +"
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash 0x00000 $(SDK_BASE)/bin/boot_v1.2.bin
endif
ifeq ($(boot), old)
	$(vecho) "Flash boot_v1.1 and +"
	$(ESPTOOL) -p $(ESPPORT) -b 256000 write_flash 0x00000 $(SDK_BASE)/bin/boot_v1.1.bin
endif
ifeq ($(boot), none)
	$(vecho) "No boot needed."
endif

flash: all
ifeq ($(app), 0)
	$(shell taskkill /f /im termite.exe)
	$(ESPTOOL) -p $(ESPPORT) -b 115200 write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x10000 $(FW_BASE)/eagle.irom0text.bin
else
ifeq ($(boot), none)
	$(ESPTOOL) -p $(ESPPORT) -b 115200 write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x10000 $(FW_BASE)/eagle.irom0text.bin
else
	$(ESPTOOL) -p $(ESPPORT) -b 115200 write_flash $(addr) $(FW_BASE)/upgrade/$(BIN_NAME).bin
endif
endif

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(ESPPORT) write_flash 0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin 0x7e000 $(SDK_BASE)/bin/blank.bin

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
