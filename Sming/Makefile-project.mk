#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

# Disable CommandExecutor functionality if not used and save some ROM and RAM.
ENABLE_CMD_EXECUTOR ?= 1

## Flash parameters
# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED ?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE ?= qio
# SPI_SIZE: 512K, 256K, 1M, 2M, 4M
SPI_SIZE ?= 512K

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Defaut debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0 
DEBUG_VERBOSE_LEVEL ?= 2

# Path to spiffy
SPIFFY ?= $(SMING_HOME)/../tools/spiffy/spiffy

#ESPTOOL2 config to generate rBootLESS images
IMAGE_MAIN	?= 0x00000.bin
IMAGE_SDK_OFFSET = $(shell printf '0x%x\n' $$(( ($$($(GET_FILESIZE) $(FW_BASE)/$(IMAGE_MAIN)) + 0x1000 + $(basename $(IMAGE_MAIN))) & (0xFFFFF000) )) )
IMAGE_SDK	?= $(IMAGE_SDK_OFFSET).bin
IROM0_ORG0       = $(shell printf '0x%x\n' $$(( 0x40200000 + $(IMAGE_SDK_OFFSET))) )

INIT_BIN_ADDR =  0x7c000
BLANK_BIN_ADDR =  0x4b000

# esptool2 path
ESPTOOL2 ?= $(SMING_HOME)/../tools/esptool2/esptool2
# esptool2 parameters for rBootLESS images
ESPTOOL2_SECTS		?= .text .data .rodata
ESPTOOL2_MAIN_ARGS	?= -quiet -bin -boot0
ESPTOOL2_SDK_ARGS	?= -quiet -lib

# SED path
SED     ?= sed

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

# Detect OS and build environment
UNAME := $(shell uname -s)

ifeq ($(OS),Windows_NT)
  # Convert Windows paths to POSIX paths
  SMING_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(SMING_HOME))))
  SMING_HOME := $(subst //,/,$(SMING_HOME))
  ESP_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(ESP_HOME))))
  ESP_HOME   := $(subst //,/,$(ESP_HOME))
endif

ifneq ($(filter MINGW32_NT%,$(UNAME)),)
  UNAME := Windows
else ifneq ($(filter CYGWIN%,$(UNAME)),)
  # Cygwin Detected
  UNAME := Linux
else ifneq ($(filter CYGWIN%WOW,$(UNAME)),)
  #Cygwin32
  UNAME := Linux
else ifneq ($(filter MSYS%WOW,$(UNAME)),)
  #Msys32
  UNAME := Linux
else ifeq ($(UNAME), Linux)
  #Linux
else ifeq ($(UNAME), Darwin)
  #OS X
else ifeq ($(UNAME), Freebsd)
  #BSD
endif

# OS specific configuration
ifeq ($(UNAME),Windows)
  # Windows detected
    
  # Default SMING_HOME. Can be overriden.
  SMING_HOME ?= c:\tools\Sming\Sming

  # Default ESP_HOME. Can be overriden.
  ESP_HOME ?= c:\Espressif

  include $(SMING_HOME)/Makefile-windows.mk
else
  ifeq ($(UNAME),Darwin)
      # MacOS Detected
      UNAME := MacOS

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /opt/sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /opt/esp-open-sdk

      include $(SMING_HOME)/Makefile-macos.mk      
  endif
  ifneq ($(filter CYGWIN%,$(UNAME)),)
      # Cygwin Detected
      UNAME := Linux
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

STRIP   := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-strip

CURRENT_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

SPIFF_FILES ?= files

BUILD_BASE	= out/build
FW_BASE		= out/firmware

# The line below calculates the next available sector after the IMAGE_SDK ROM.
# The math is the following: next_available_sector_start = (( address + size ) + one_sector_size) & 0xFFFFF000
# 0x01000    is the size of one sector
# IMAGE_SDK  contains starting address of the irom0 section and its size
# 0xFFFF000  mask that is used to show the start of a sector
SPIFF_START_OFFSET = $(shell printf '0x%X\n' $$(( ($$($(GET_FILESIZE) $(FW_BASE)/$(IMAGE_SDK)) + 0x1000 + $(basename $(IMAGE_SDK))) & (0xFFFFF000) )) )

#Firmware memory layout info files
FW_MEMINFO_NEW = $(FW_BASE)/fwMeminfo.new
FW_MEMINFO_OLD = $(FW_BASE)/fwMeminfo.old
FW_MEMINFO_SAVED = out/fwMeminfo

# name for the target project
TARGET		= app

THIRD_PARTY_DIR = $(SMING_HOME)/third-party

SMING_FEATURES = none
LIBSMING = sming
ifeq ($(ENABLE_SSL),1)
	LIBSMING = smingssl
	SMING_FEATURES = SSL
endif

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES      ?= app     # default to app if not set by user
EXTRA_INCDIR ?= include # default to include if not set by user

ENABLE_CUSTOM_LWIP ?= 1
LWIP_INCDIR = $(SMING_HOME)/system/esp-lwip/lwip/include
ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	LWIP_INCDIR = $(SMING_HOME)/third-party/esp-open-lwip/include	
else ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	LWIP_INCDIR = $(SMING_HOME)/third-party/lwip2/glue-esp/include-esp  $(SMING_HOME)/third-party/lwip2/include
endif

EXTRA_INCDIR += $(SMING_HOME)/include $(SMING_HOME)/ $(LWIP_INCDIR) $(SMING_HOME)/system/include \
				$(SMING_HOME)/Wiring $(SMING_HOME)/Libraries \
				$(SMING_HOME)/Libraries/Adafruit_GFX $(SMING_HOME)/Libraries/Adafruit_Sensor \
				$(SMING_HOME)/SmingCore $(SMING_HOME)/Services/SpifFS $(SDK_BASE)/../include \
				$(THIRD_PARTY_DIR)/rboot $(THIRD_PARTY_DIR)/rboot/appcode $(THIRD_PARTY_DIR)/spiffs/src

ENABLE_CUSTOM_HEAP ?= 0
 
USER_LIBDIR = $(SMING_HOME)/compiler/lib/
 
LIBMAIN = main
ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN = mainmm
endif

LIBLWIP = lwip
ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	LIBLWIP = lwip_open
	ifeq ($(ENABLE_ESPCONN), 1)
		LIBLWIP = lwip_full
	endif
	CUSTOM_TARGETS += $(USER_LIBDIR)/lib$(LIBLWIP).a 
endif
ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	ifeq ($(ENABLE_ESPCONN), 1)
$(error LWIP2 does not support espconn_* functions. Make sure to set ENABLE_CUSTOM_LWIP to 0 or 1.)
	endif
	LIBLWIP = lwip2
	CUSTOM_TARGETS += $(USER_LIBDIR)/liblwip2.a 
endif

LIBPWM = pwm

ENABLE_CUSTOM_PWM ?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	LIBPWM = pwm_open
	CUSTOM_TARGETS += $(USER_LIBDIR)/lib$(LIBPWM).a
endif

# libraries used in this project, mainly provided by the SDK
LIBS		= microc microgcc hal phy pp net80211 $(LIBLWIP) wpa $(LIBSMING) $(LIBMAIN) crypto $(LIBPWM) smartconfig $(EXTRA_LIBS)
ifeq ($(ENABLE_WPS),1)
	LIBS += wps
endif

# compiler flags using during compilation of source files
CFLAGS  = -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections \
          -D__ets__ -DICACHE_FLASH -DARDUINO=106 -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL) $(USER_CFLAGS) -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR) -DSMING_INCLUDED=1
# => SDK
ifneq (,$(findstring third-party/ESP8266_NONOS_SDK, $(SDK_BASE)))
	CFLAGS += -DSDK_INTERNAL
endif
ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS += -Os -DSMING_RELEASE=1
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -Og -ggdb -DGDBSTUB_FREERTOS=0 -DENABLE_GDB=1
	MODULES		 += $(THIRD_PARTY_DIR)/gdbstub
	EXTRA_INCDIR += $(THIRD_PARTY_DIR)/gdbstub
	STRIP := @true
else
	CFLAGS += -Os -g
	STRIP := @true
endif
ifeq ($(ENABLE_WPS),1)
	CFLAGS += -DENABLE_WPS=1
endif

#Append debug options
CFLAGS += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)

CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

# SSL support using axTLS
ifeq ($(ENABLE_SSL),1)
	LIBS += axtls	
	EXTRA_INCDIR += $(THIRD_PARTY_DIR)/axtls-8266 $(THIRD_PARTY_DIR)/axtls-8266/ssl $(THIRD_PARTY_DIR)/axtls-8266/crypto 
	AXTLS_FLAGS = -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1) # 
		AXTLS_FLAGS += -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1
	endif
	
	CUSTOM_TARGETS += include/ssl/private_key.h
	CFLAGS += $(AXTLS_FLAGS)  
	CXXFLAGS += $(AXTLS_FLAGS)	
endif

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
LDFLAGS		= -nostdlib -u call_user_start -u custom_crash_callback -Wl,-static -Wl,--gc-sections -Wl,-Map=$(FW_BASE)/firmware.map -Wl,-wrap,system_restart_local 

# linker script used for the above linkier step
LD_PATH     = $(SMING_HOME)/compiler/ld
PROJECT_LD_PATH=ld

LD_SCRIPT	= standalone.rom.ld

ifeq ($(SPI_SPEED), 26)
	flashimageoptions = -ff 26m
else ifeq ($(SPI_SPEED), 20)
	flashimageoptions = -ff 20m
else ifeq ($(SPI_SPEED), 80)
	flashimageoptions = -ff 80m
else
	flashimageoptions = -ff 40m
endif

ifeq ($(SPI_MODE), qout)
	flashimageoptions += -fm qout
else ifeq ($(SPI_MODE), dio)
	flashimageoptions += -fm dio
else ifeq ($(SPI_MODE), dout)
	flashimageoptions += -fm dout
else
	flashimageoptions += -fm qio
endif

# flash larger than 1024KB only use 1024KB to storage user1.bin and user2.bin
ifeq ($(SPI_SIZE), 256K)
	flashimageoptions += -fs 2m
	SPIFF_SIZE ?= 131072  #128K
else ifeq ($(SPI_SIZE), 1M)
	flashimageoptions += -fs 8m
	SPIFF_SIZE ?= 524288  #512K
	INIT_BIN_ADDR  = 0x0fc000
	BLANK_BIN_ADDR = 0x0fe000
else ifeq ($(SPI_SIZE), 2M)
	flashimageoptions += -fs 16m
	SPIFF_SIZE ?= 524288  #512K
	INIT_BIN_ADDR  = 0x1fc000
	BLANK_BIN_ADDR = 0x1fe000
else ifeq ($(SPI_SIZE), 4M)
	flashimageoptions += -fs 32m
	SPIFF_SIZE ?= 524288  #512K
	INIT_BIN_ADDR  = 0x3fc000
	BLANK_BIN_ADDR = 0x3fe000
else
	flashimageoptions += -fs 4m
	SPIFF_SIZE ?= 196608  #192K
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

C_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
CXX_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))

C_OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(C_SRC))
CXX_OBJ		:= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(CXX_SRC))
AS_OBJ		:= $(patsubst %.s,$(BUILD_BASE)/%.o,$(AS_SRC))

OBJ		:= $(AS_OBJ) $(C_OBJ) $(CXX_OBJ)

LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

SPIFF_BIN_OUT ?= spiff_rom
SPIFF_BIN_OUT := $(FW_BASE)/$(SPIFF_BIN_OUT).bin

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


define compile-objects
${BUILD_BASE}/$1/%.o: $1/%.c ${BUILD_BASE}/$1/%.c.d
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@	
${BUILD_BASE}/$1/%.o: $1/%.cpp ${BUILD_BASE}/$1/%.cpp.d
	$(vecho) "C+ $$<" 
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS) -c $$< -o $$@
${BUILD_BASE}/$1/%.c.d: $1/%.c
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -MM -MT $1/$$*.o $$< -o $$@
${BUILD_BASE}/$1/%.cpp.d: $1/%.cpp
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS) -MM -MT $1/$$*.o $$< -o $$@

.PRECIOUS: ${BUILD_BASE}/$1/%.c.d ${BUILD_BASE}/$1/%.cpp.d
endef

.PHONY: all checkdirs spiff_update spiff_clean clean

all: $(USER_LIBDIR)/lib$(LIBSMING).a checkdirs $(TARGET_OUT) $(SPIFF_BIN_OUT) $(FW_FILE_1) $(FW_FILE_2)

spiff_update: spiff_clean $(SPIFF_BIN_OUT)

$(PROJECT_LD_PATH)/$(LD_SCRIPT):
	$(Q) mkdir -p $(PROJECT_LD_PATH)
	$(Q) cp $(LD_PATH)/$(LD_SCRIPT) $@ 
	
$(FW_BASE)/$(IMAGE_MAIN): $(APP_AR)
# Pass 1: Generate rom0 to be able to check its size
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(LD_PATH) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $(TARGET_OUT).tmp
	
	$(Q) $(STRIP) $(TARGET_OUT).tmp

	$(Q) $(ESPTOOL2) $(ESPTOOL2_MAIN_ARGS) $(TARGET_OUT).tmp $@ $(ESPTOOL2_SECTS)
	
	$(Q) rm $(TARGET_OUT).tmp

$(TARGET_OUT): $(FW_BASE)/$(IMAGE_MAIN) $(PROJECT_LD_PATH)/$(LD_SCRIPT)
	$(vecho) "LD $@"
	
# Readjust linker
	$(Q) $(SED) -r "s/(^\s*irom0_0_seg *: *).*/\\1org = $(IROM0_ORG0), len = \(1M - $(IMAGE_SDK_OFFSET)\)/" $(LD_PATH)/$(LD_SCRIPT) > $(PROJECT_LD_PATH)/$(LD_SCRIPT)
	
# Pass 2: Generate roms with correct offsets
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(PROJECT_LD_PATH) -L$(LD_PATH) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

	$(Q) $(STRIP) $@

	$(vecho) ""	
	$(vecho) "#Memory / Section info:"	
	$(vecho) "------------------------------------------------------------------------------"
#Check for existing old meminfo file and move it to /out/firmware as the infofile from previous build
	$(Q) if [ -f "$(FW_MEMINFO_SAVED)" ]; then \
	  mv $(FW_MEMINFO_SAVED) $(FW_MEMINFO_OLD); \
	fi
	
	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)
	
	$(Q) if [ -f "$(FW_MEMINFO_NEW)" -a -f "$(FW_MEMINFO_OLD)" ]; then \
	  awk -F "|" 'FILENAME == "$(FW_MEMINFO_OLD)" { arr[$$1]=$$5 } FILENAME == "$(FW_MEMINFO_NEW)" { if (arr[$$1] != $$5){printf "%s (%+d)\n", $$0,$$5 - arr[$$1] } else {printf "%s\n", $$0} }' $(FW_MEMINFO_OLD) $(FW_MEMINFO_NEW); \
	elif [ -f "$(FW_MEMINFO_NEW)" ]; then \
	  cat $(FW_MEMINFO_NEW); \
	fi

	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "# Generating image..."
#	$(Q) $(ESPTOOL2) elf2image $@ $(flashimageoptions) -o $(FW_BASE)/
	$(Q) $(ESPTOOL2) $(ESPTOOL2_MAIN_ARGS) $@ $(FW_BASE)/$(IMAGE_MAIN) $(ESPTOOL2_SECTS)
	$(Q) $(ESPTOOL2) $(ESPTOOL2_SDK_ARGS) $@ $(FW_BASE)/$(IMAGE_SDK)
	$(vecho) "Generate firmware images successully in folder $(FW_BASE)."
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) test ! -f $@ || rm $@
	$(Q) $(AR) rcsP $@ $^

$(USER_LIBDIR)/lib$(LIBSMING).a:
	$(vecho) "(Re)compiling Sming. Enabled features: $(SMING_FEATURES). This may take some time"
	$(Q) $(MAKE) -C $(SMING_HOME) clean V=$(V) ENABLE_SSL=$(ENABLE_SSL) SMING_HOME=$(SMING_HOME)
	$(Q) $(MAKE) -C $(SMING_HOME) V=$(V) ENABLE_SSL=$(ENABLE_SSL) SMING_HOME=$(SMING_HOME)

include/ssl/private_key.h:
	$(vecho) "Generating unique certificate and key. This may take some time"
	$(Q) mkdir -p $(CURRENT_DIR)/include/ssl/
	$(Q) AXDIR=$(CURRENT_DIR)/include/ssl/  $(THIRD_PARTY_DIR)/axtls-8266/tools/make_certs.sh 

ifeq ($(ENABLE_CUSTOM_PWM), 1)
$(USER_LIBDIR)/libpwm_open.a:
	$(Q) $(MAKE) -C $(SMING_HOME) compiler/lib/libpwm_open.a ENABLE_CUSTOM_PWM=1
endif

$(USER_LIBDIR)/liblwip%.a:
	$(Q) $(MAKE) -C $(SMING_HOME) compiler/lib/$(notdir $@) \
				ENABLE_CUSTOM_LWIP=$(ENABLE_CUSTOM_LWIP) \
				ENABLE_ESPCONN=$(ENABLE_ESPCONN)

checkdirs: $(BUILD_DIR) $(FW_BASE) $(CUSTOM_TARGETS)

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
		echo "Creating empty $(SPIFF_BIN_OUT)"; \
		$(SPIFFY) $(SPIFF_SIZE) dummy.dir $(SPIFF_BIN_OUT); \
	fi
	$(vecho) "$(SPIFF_BIN_OUT)---------->$(SPIFF_START_OFFSET)"
endif

flash: all
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)
ifeq ($(DISABLE_SPIFFS), 1)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(basename $(IMAGE_MAIN)) $(FW_BASE)/$(IMAGE_MAIN) $(IMAGE_SDK_OFFSET) $(FW_BASE)/$(IMAGE_SDK)
else
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(basename $(IMAGE_MAIN)) $(FW_BASE)/$(IMAGE_MAIN) $(IMAGE_SDK_OFFSET) $(FW_BASE)/$(IMAGE_SDK) $(SPIFF_START_OFFSET) $(SPIFF_BIN_OUT)
endif
	$(TERMINAL)

terminal:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)
	$(TERMINAL)

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) erase_flash
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(INIT_BIN_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin $(BLANK_BIN_ADDR) $(SDK_BASE)/bin/blank.bin $(SPIFF_START_OFFSET) $(SMING_HOME)/compiler/data/blankfs.bin

rebuild: clean all

clean:
#preserve meminfo file from /out/firmware to /out/
	$(Q) if [ -f "$(FW_MEMINFO_NEW)" ]; then \
		mv $(FW_MEMINFO_NEW) $(FW_MEMINFO_SAVED); \
	fi
#remove build artifacts
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach mod,$(MODULES),$(eval $(call compile-objects,$(mod))))
$(foreach bdir,$(BUILD_DIR),$(eval include $(wildcard $(bdir)/*.c.d)))
$(foreach bdir,$(BUILD_DIR),$(eval include $(wildcard $(bdir)/*.cpp.d)))
