#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

SERVER_OTA_PORT ?= 9999

# rBoot options, overwrite them in the projects Makefile-user.mk
RBOOT_BIG_FLASH  ?= 1
RBOOT_TWO_ROMS   ?= 0
RBOOT_RTC_ENABLED ?= 0
RBOOT_GPIO_ENABLED ?= 0
# RBOOT_GPIO_SKIP_ENABLED and RBOOT_GPIO_ENABLED cannot be used at the same time.
RBOOT_GPIO_SKIP_ENABLED ?= 0

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

RBOOT_ROM_0      ?= rom0
RBOOT_ROM_1      ?= rom1
RBOOT_SPIFFS_0   ?= 0x100000
RBOOT_SPIFFS_1   ?= 0x300000
RBOOT_LD_0 ?= rboot.rom0.ld
RBOOT_LD_1 ?= rom1.ld
# esptool2 path
ESPTOOL2 ?= $(SMING_HOME)/../tools/esptool2/esptool2
# path to spiffy
SPIFFY ?= $(SMING_HOME)/../tools/spiffy/spiffy
INIT_BIN_ADDR  = 0x7c000
BLANK_BIN_ADDR = 0x4b000
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

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Defaut debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0 
DEBUG_VERBOSE_LEVEL ?= 2

# Disable CommandExecutor functionality if not used and save some ROM and RAM
ENABLE_CMD_EXECUTOR ?= 1

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

#Firmware memory layout info files
FW_MEMINFO_NEW = $(FW_BASE)/fwMeminfo.new
FW_MEMINFO_OLD = $(FW_BASE)/fwMeminfo.old
FW_MEMINFO_SAVED = out/fwMeminfo

RBOOT_ROM_0  := $(addprefix $(FW_BASE)/,$(RBOOT_ROM_0).bin)
RBOOT_ROM_1  := $(addprefix $(FW_BASE)/,$(RBOOT_ROM_1).bin)
RBOOT_SILENT ?= 0

# name for the target project
TARGET		= app

THIRD_PARTY_DIR = $(SMING_HOME)/third-party

LIBSMING = sming
SMING_FEATURES = none
ifeq ($(ENABLE_SSL),1)
	LIBSMING = smingssl
	SMING_FEATURES = SSL
endif

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES      ?= app     # default to app if not set by user
MODULES      += $(THIRD_PARTY_DIR)/rboot/appcode
MODULES      += $(SMING_HOME)/appspecific/rboot
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

USER_LIBDIR  = $(SMING_HOME)/compiler/lib/

# compiler flags using during compilation of source files
CFLAGS   = -Wall -Wundef -Wpointer-arith -Wno-comment -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections \
           -D__ets__ -DICACHE_FLASH -DARDUINO=106 -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL) $(USER_CFLAGS) -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR) -DSMING_INCLUDED=1 
ifneq ($(STRICT),1)
CFLAGS += -Werror -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-return-type -Wno-maybe-uninitialized
endif

# => SDK
ifneq (,$(findstring third-party/ESP8266_NONOS_SDK, $(SDK_BASE)))
	CFLAGS += -DSDK_INTERNAL
endif
ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS += -Os -DSMING_RELEASE=1
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -Og -ggdb -DGDBSTUB_FREERTOS=0 -DENABLE_GDB=1 -DGDBSTUB_CTRLC_BREAK=0
	MODULES		 += $(THIRD_PARTY_DIR)/esp-gdbstub
	EXTRA_INCDIR += $(THIRD_PARTY_DIR)/esp-gdbstub
	STRIP := @true
else
	CFLAGS += -Os -g
	STRIP := @true
endif
ifeq ($(ENABLE_WPS),1)
   CFLAGS += -DENABLE_WPS=1
endif

# Flags for compatability with old versions (most of them should disappear with the next major release)
ifeq ($(MQTT_NO_COMPAT),1)
	CFLAGS += -DMQTT_NO_COMPAT=1
endif

#Append debug options
CFLAGS  += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

ENABLE_CUSTOM_HEAP ?= 0
 
LIBMAIN = main
ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN = mainmm
endif

LIBMAIN = main
LIBMAIN_SRC = $(addprefix $(SDK_LIBDIR)/,libmain.a)

ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN = mainmm
	LIBMAIN_SRC := $(USER_LIBDIR)lib$(LIBMAIN).a
endif

# libmain must be modified for rBoot big flash support (just one symbol gets weakened)
ifeq ($(RBOOT_BIG_FLASH),1)
	LIBMAIN = main2
	LIBMAIN_DST = $(addprefix $(BUILD_BASE)/,libmain2.a)
	CFLAGS += -DBOOT_BIG_FLASH
else
	LIBMAIN_DST = $()
endif
# libraries used in this project, mainly provided by the SDK

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

LIBS		= microc microgcc hal phy pp net80211 $(LIBLWIP) mqttc wpa $(LIBSMING) $(LIBMAIN) crypto $(LIBPWM) smartconfig $(EXTRA_LIBS)
ifeq ($(ENABLE_WPS),1)
   LIBS += wps
endif

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

ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	EXTRA_INCDIR += third-party/esp-open-lwip/include
endif
ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	EXTRA_INCDIR += third-party/lwip2/include
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
LDFLAGS		= -nostdlib -u call_user_start -u Cache_Read_Enable_New -u spiffs_get_storage_config -u custom_crash_callback -Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map -Wl,-wrap,system_restart_local 

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

C_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
CXX_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))

C_OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(C_SRC))
CXX_OBJ		:= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(CXX_SRC))
AS_OBJ		:= $(patsubst %.s,$(BUILD_BASE)/%.o,$(AS_SRC))

OBJ		:= $(AS_OBJ) $(C_OBJ) $(CXX_OBJ)

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
	RBOOT_ROM_1 := $()
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

all: $(USER_LIBDIR)/lib$(LIBSMING).a checkdirs $(LIBMAIN_DST) $(RBOOT_BIN) $(RBOOT_ROM_0) $(RBOOT_ROM_1) $(SPIFF_BIN_OUT) $(FW_FILE_1) $(FW_FILE_2) 

$(RBOOT_BIN):
	$(MAKE) -C $(THIRD_PARTY_DIR)/rboot RBOOT_GPIO_ENABLED=$(RBOOT_GPIO_ENABLED) RBOOT_SILENT=$(RBOOT_SILENT)

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
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(SMING_HOME)/compiler/ld $(RBOOT_LD_0) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@
	$(Q) $(STRIP) $@

	$(Q) $(MEMANALYZER) $@ > $(FW_MEMINFO_NEW)

	$(Q) if [ -f "$(FW_MEMINFO_NEW)" -a -f "$(FW_MEMINFO_OLD)" ]; then \
	  awk -F "|" 'FILENAME == "$(FW_MEMINFO_OLD)" { arr[$$1]=$$5 } FILENAME == "$(FW_MEMINFO_NEW)" { if (arr[$$1] != $$5){printf "%s%s%+d%s", substr($$0, 1, length($$0) - 1)," (",$$5 - arr[$$1],")\n" } else {print $$0} }' $(FW_MEMINFO_OLD) $(FW_MEMINFO_NEW); \
	elif [ -f "$(FW_MEMINFO_NEW)" ]; then \
	  cat $(FW_MEMINFO_NEW); \
	fi


$(TARGET_OUT_1): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(USER_LIBDIR) -L$(SDK_LIBDIR) -L$(BUILD_BASE) -L$(SMING_HOME)/compiler/ld  $(RBOOT_LD_1) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@
	$(Q) $(STRIP) $@

# recreate it from 0, since you get into problems with same filenames
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
endif

flashboot: $(USER_LIBDIR)/lib$(LIBSMING).a $(RBOOT_BIN)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x00000 $(RBOOT_BIN)
	
flashconfig:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)
	$(vecho) "Deleting rBoot config sector"
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x01000 $(SDK_BASE)/bin/blank.bin 

flashapp: all
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x02000 $(RBOOT_ROM_0)

flashfs: $(USER_LIBDIR)/lib$(LIBSMING).a $(SPIFF_BIN_OUT)
ifeq ($(DISABLE_SPIFFS), 1)
	$(vecho) "SPIFFS are not enabled!"
else
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(RBOOT_SPIFFS_0) $(SPIFF_BIN_OUT)
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
	
otaserver: all
	$(vecho) "Starting OTA server for TESTING"
	$(Q) cd $(FW_BASE) && python -m SimpleHTTPServer $(SERVER_OTA_PORT)

terminal:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)
	$(TERMINAL)

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) erase_flash
ifeq ($(DISABLE_SPIFFS), 1)
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(INIT_BIN_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin $(BLANK_BIN_ADDR) $(SDK_BASE)/bin/blank.bin
else
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(INIT_BIN_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin $(BLANK_BIN_ADDR) $(SDK_BASE)/bin/blank.bin $(RBOOT_SPIFFS_0) $(SMING_HOME)/compiler/data/blankfs.bin
endif

rebuild: clean all

clean:
#remove build artifacts
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach mod,$(MODULES),$(eval $(call compile-objects,$(mod))))
$(foreach bdir,$(BUILD_DIR),$(eval include $(wildcard $(bdir)/*.c.d)))
$(foreach bdir,$(BUILD_DIR),$(eval include $(wildcard $(bdir)/*.cpp.d)))
