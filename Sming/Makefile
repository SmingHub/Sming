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

# Git command
GIT ?= git


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
  
  # Remove possible double escaping...
  SMING_HOME := $(subst //,/, $(SMING_HOME))
  ESP_HOME   := $(subst //,/, $(ESP_HOME))

  include Makefile-windows.mk  
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME),Darwin)
      # MacOS Detected
      UNAME := MacOS

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /opt/sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /opt/esp-open-sdk

      include Makefile-macos.mk      
  endif
  ifeq ($(UNAME),Linux)
      # Linux Detected
      UNAME := Linux

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /opt/sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /opt/esp-open-sdk
      include Makefile-linux.mk     
  endif
  ifeq ($(UNAME),FreeBSD)
      # Freebsd Detected
      UNAME := FreeBSD

      # Default SMING_HOME. Can be overriden.
      SMING_HOME ?= /usr/local/esp8266/Sming/Sming

      # Default ESP_HOME. Can be overriden.
      ESP_HOME ?= /usr/local/esp8266/esp-open-sdk
      include Makefile-bsd.mk
  endif
endif

export ESP_HOME
export SMING_HOME
export COMPILE := gcc
export PATH := $(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)
XTENSA_TOOLS_ROOT := $(ESP_HOME)/xtensa-lx106-elf/bin

# select which tools to use as compiler, librarian and linker
AS		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CXX		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Defaut debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0 
DEBUG_VERBOSE_LEVEL ?= 2

# Disable CommandExecutor functionality if not used and save some ROM and RAM
ENABLE_CMD_EXECUTOR ?= 1

RELOAD_MKFILE = 0

# Sming Framework Path
SMF = $(SMING_HOME)

SPIFF_FILES = files

BUILD_BASE	= out/build
FW_BASE		= out/firmware
USER_LIBDIR = compiler/lib

LIBSMING = libsming
ifeq ($(ENABLE_SSL),1)
	LIBSMING= libsmingssl
endif


# name for the target project
TARGET		= app


CUSTOM_TARGETS ?=

# which modules (subdirectories) of the project to include in compiling
MODULES		= system system/helpers Wiring SmingCore appinit $(filter %/, $(wildcard SmingCore/*/)) $(filter %/, $(wildcard Services/*/)) $(filter %/, $(wildcard Libraries/*/))
EXTRA_INCDIR    = include system/include Wiring Libraries SmingCore $(SDK_BASE)/../include

# Place a file that should exist in a submodule that is fetched separately
# => rboot 
THIRD_PARTY_DATA = third-party/rboot/Makefile
EXTRA_INCDIR += third-party/rboot third-party/rboot/appcode 

# => spiffs
THIRD_PARTY_DATA += third-party/spiffs/makefile
MODULES		     += third-party/spiffs/src
EXTRA_INCDIR     += third-party/spiffs/src

# => http-parser
THIRD_PARTY_DATA += third-party/http-parser/Makefile
MODULES		     += third-party/http-parser/
EXTRA_INCDIR     += third-party/http-parser/

# => esp-gdbstub
ifeq ($(ENABLE_GDB), 1)
	THIRD_PARTY_DATA += third-party/esp-gdbstub/Makefile
	MODULES		 += third-party/esp-gdbstub
	EXTRA_INCDIR += third-party/esp-gdbstub
else ifneq ($(SMING_RELEASE),1)
	MODULES += gdb
endif

# => umm_malloc (custom heap allocation)
ifeq ($(ENABLE_CUSTOM_HEAP), 1)
	THIRD_PARTY_DATA += third-party/umm_malloc/Makefile
	MODULES      += custom_heap third-party/umm_malloc/src
	EXTRA_INCDIR += third-party/umm_malloc/src third-party/umm_malloc/includes/c-helper-macros
	CUSTOM_TARGETS += $(USER_LIBDIR)/libmainmm.a
endif

# => Open Source LWIP
LIBLWIP = lwip
ENABLE_CUSTOM_LWIP ?= 1
ENABLE_ESPCONN ?= 0
ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	THIRD_PARTY_DATA += third-party/esp-open-lwip/Makefile.open
	EXTRA_INCDIR += third-party/esp-open-lwip/include
	EXTRA_CFLAGS_LWIP = -I../../system/include -I../../Wiring
	ENABLE_LWIPDEBUG ?= 0
	ifeq ($(ENABLE_LWIPDEBUG), 1)
		EXTRA_CFLAGS_LWIP += -DLWIP_DEBUG
	endif

	ifeq ($(ENABLE_ESPCONN), 1)
		LIBLWIP = lwip_full
	else
		LIBLWIP = lwip_open
	endif
	CUSTOM_TARGETS += $(USER_LIBDIR)/lib$(LIBLWIP).a
endif

# libraries used in this project, mainly provided by the SDK
LIBS		= microc microgcc hal phy pp net80211 $(LIBLWIP) wpa main
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	THIRD_PARTY_DATA += third-party/pwm/pwm.c
endif

MFORCE32 := $(shell $(CC) --help=target | grep mforce-l32)

# compiler flags using during compilation of source files. Add '-pg' for debugging
CFLAGS		= -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections -D__ets__ -DICACHE_FLASH -DARDUINO=106 -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL) -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR)
ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS += -Os -DSMING_RELEASE=1
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -Og -ggdb -DGDBSTUB_FREERTOS=0 -DENABLE_GDB=1
else
	CFLAGS += -Os -g
endif
ifneq ($(MFORCE32),)
    # Your compiler supports the -mforce-l32 flag which means that 
    # constants can be stored in flash (program) memory instead of SRAM.
    # See: https://www.arduino.cc/en/Reference/PROGMEM
    CFLAGS += -DPROGMEM_L32="__attribute__((aligned(4))) __attribute__((section(\".irom.text\")))" -mforce-l32
else 
	CFLAGS += -DPROGMEM_L32=""
endif
#Append debug options
CFLAGS += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)

CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -u call_user_start -Wl,-static -Wl,--gc-sections -Wl,-wrap,system_restart_local 

# linker script used for the above linkier step
LD_PATH     = compiler/ld/
LD_SCRIPT	= $(LD_PATH)eagle.app.v6.cpp.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# SSL support using axTLS
ifeq ($(ENABLE_SSL),1)
	THIRD_PARTY_DATA += third-party/axtls-8266/Makefile
	LIBS += axtls
	MODULES		 += third-party/axtls-8266/compat third-party/axtls-8266/replacements
	EXTRA_INCDIR += third-party/axtls-8266 axtls-8266/ssl third-party/axtls-8266/crypto 
	AXTLS_FLAGS = -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1) # 
		AXTLS_FLAGS += -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1 -DAXL_DEBUG=1
	endif
	CUSTOM_TARGETS += $(USER_LIBDIR)/libaxtls.a
	EXTRA_CFLAGS_AXTLS = -I../../system/include -I../../Wiring
	CFLAGS += $(AXTLS_FLAGS)  
	CXXFLAGS += $(AXTLS_FLAGS)
endif

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

AS_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
ifeq ($(ENABLE_GDB), 1)
	AS_SRC += $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.S)) 
endif
C_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
CXX_SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
AS_OBJ		:= $(patsubst %.s,$(BUILD_BASE)/%.o,$(AS_SRC))
ifeq ($(ENABLE_GDB), 1)
	AS_OBJ		+= $(patsubst %.S,$(BUILD_BASE)/%.o,$(AS_SRC))
endif
C_OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(C_SRC))
CXX_OBJ		:= $(patsubst %.cpp,$(BUILD_BASE)/%.o,$(CXX_SRC))
OBJ		:= $(AS_OBJ) $(C_OBJ) $(CXX_OBJ)
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
LD_SCRIPT	:= $(addprefix -T,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

SAMPLES_DIRS := $(shell ls -1 ../samples)
THIRD_PARTY_DIRS := $(shell ls -1 third-party )
DOXYGEN := $(shell command -v doxygen 2> /dev/null)

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
vpath %.s $(SRC_DIR)
ifeq ($(ENABLE_GDB), 1)
	vpath %.S $(SRC_DIR)
endif

define compile-objects
$1/%.o: %.s
	$(vecho) "AS $$<"
	$(Q) $(AS) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
ifeq ($(ENABLE_GDB), 1)	
$1/%.o: %.S
	$(vecho) "AS $$<"
	$(Q) $(AS) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endif	
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
$1/%.o: %.cpp
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean spiffy test samples-clean samples $(SAMPLES_DIRS) docs api wiki

all: checkdirs $(APP_AR) spiffy

ifeq ($(ENABLE_CUSTOM_PWM), 1)	
$(USER_LIBDIR)/libpwm_open.a: third-party/pwm/pwm.c
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $< -o $(dir $<)/pwm.o
	$(Q) $(AR) rcs $@ $(dir $<)/pwm.o
endif 

ifeq ($(ENABLE_SSL), 1)
$(USER_LIBDIR)/libaxtls.a:
	$(Q) $(MAKE) -C third-party/axtls-8266 -e V=$(V) BIN_DIR="$(SMING_HOME)/$(USER_LIBDIR)" CFLAGS_EXTRA="$(EXTRA_CFLAGS_AXTLS)"
endif

ifeq ($(ENABLE_CUSTOM_HEAP), 1)
$(USER_LIBDIR)/libmainmm.a: $(addprefix $(SDK_LIBDIR)/,libmain.a)
	$(vecho) "Enabling custom heap implementation"
	$(Q) cp $^ $@ 
	$(Q) $(AR) -d $@ mem_manager.o
endif

ifeq ($(ENABLE_CUSTOM_LWIP), 1)
$(USER_LIBDIR)/liblwip_%.a: third-party/esp-open-lwip/Makefile.open
	$(Q) $(MAKE) -C third-party/esp-open-lwip/ -f Makefile.open ENABLE_ESPCONN=$(ENABLE_ESPCONN) SDK_BASE="$(SDK_BASE)" USER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)/" CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP)" all
endif

spiffy: spiffy/spiffy

spiffy/spiffy: third-party/spiffs/makefile
	$(vecho) "Making spiffy utility"
	$(Q) $(MAKE) --no-print-directory -C spiffy V=$(V)
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^
	$(vecho) "Installing $(LIBSMING)"
ifeq ($(ENABLE_SSL), 1)
	$(vecho) "+ SSL support is enabled"
else
	$(vecho) "(!) SSL support is not enabled. To enable it type: 'make clean; make ENABLE_SSL=1'"
endif
	$(Q) cp -r $(APP_AR) $(USER_LIBDIR)/$(LIBSMING).a  
	$(vecho) "Done"

checkdirs: $(THIRD_PARTY_DATA) reload $(BUILD_DIR) $(FW_BASE) $(CUSTOM_TARGETS)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@
	$(Q) mkdir -p $@/upgrade

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)
ifeq ($(ENABLE_SSL), 1)
	$(Q) -$(MAKE) --no-print-directory -C third-party/axtls-8266 clean -e V=$(V) BIN_DIR="$(SMING_HOME)/$(USER_LIBDIR)" CFLAGS_EXTRA="$(EXTRA_CFLAGS_AXTLS)"
else
	$(Q) $(MAKE) --no-print-directory -C spiffy clean V=$(V)
endif	
ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	$(Q) -$(MAKE) -C third-party/esp-open-lwip/ -f Makefile.open ENABLE_ESPCONN=$(ENABLE_ESPCONN) SDK_BASE="$(SDK_BASE)" USER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)/" CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP)" clean
endif

test: Basic_Blink Basic_rBoot Basic_Ssl Basic_HwPWM 

docs/wiki/Home.md: 
	$(vecho) "No Wiki submodule found. Using git to fetch it..."
	$(Q) $(GIT) submodule update --init ../docs/wiki	

docs/api/error.log:
ifndef DOXYGEN
	$(error doxygen not found - not building API docs)
endif
	$(Q) mkdir -p $(SMING_HOME)/../docs/api
	$(Q) cd $(SMING_HOME)/../docs; doxygen 2>$(SMING_HOME)/../docs/api/stderr.log 1>$(SMING_HOME)/../docs/api/doxygen.log

docs/api/sming/index.html: docs/api/error.log
	$(vecho) "Undocumented compounds: $(shell grep "Compound.*is not documented" $(SMING_HOME)/../docs/api/error.log | wc -l)"
	$(vecho) "Undocumented members:  $(shell grep "Member.*is not documented" $(SMING_HOME)/../docs/api/error.log | wc -l) "
	$(vecho) "Undocumented parameters: $(shell grep "The following parameters of .* are not documented" $(SMING_HOME)/../docs/api/error.log | wc -l) "
	$(vecho) "Open in your browser: file://$(SMING_HOME)/../$@"

wiki: docs/wiki/Home.md 
api: docs/api/sming/index.html 

docs: wiki api

third-party/%: 
	$(vecho) "Fetching $(dir $@) ..."
	$(Q) $(GIT) submodule update --init --recursive $(dir $@)
	$(Q) touch $(patsubst third-party/%/,third-party/.patches/%.patch, $(dir $@))
	$(Q) -cd $(dir $@); $(GIT) apply -v $(patsubst third-party/%/,$(SMING_HOME)/third-party/.patches/%.patch, $(dir $@)) --ignore-whitespace --whitespace=nowarn
# if the new submodule brings source code files that need to be compiled inside Sming
# then we need somehow to be able to "see" these new files. 
# For now we solve this by "reloading" the makefile after fetching a module.
    RELOAD_MKFILE=1

reload: 
	$(Q) if [ $(RELOAD_MKFILE) -eq 1 ]; then \
        $(MAKE) -C $(SMING_HOME) $(MAKECMDGOALS) RELOAD_MKFILE=0; \
    fi

samples: $(SAMPLES_DIRS)

$(SAMPLES_DIRS):
	$(Q) $(MAKE) -C $(SMING_HOME)/../samples/$@

samples-clean:
	$(Q) for dir in $(SAMPLES_DIRS); do \
          $(MAKE) -C $(SMING_HOME)/../samples/$$dir clean; \
        done
        
third-party-clean:
	$(Q) for dir in $(THIRD_PARTY_DIRS); do \
          rm -rf third-party/$$dir; \
        done
	$(Q) $(GIT) checkout third-party     
       
dist-clean: clean samples-clean third-party-clean
	$(Q) for file in $(shell ls $(USER_LIBDIR)/lib*.a ); do \
 	   rm $$file; \
 	done
	$(Q) $(GIT) checkout $(USER_LIBDIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))

