#   sming.mk
#
#   done:
#       memanalyzer
#       improved windows terminal handling
#       automatic address caclulation and linker script creation
#       dependency tracking for spiffs sources
#       dependency tracking for cpp and c sources
#
#==============================================================================
#   check user settings
#------------------------------------------------------------------------------
ifeq ("$(SPI_SIZE)","")
    $(error SPI_SIZE not set)
endif	


#==============================================================================
#   os specific settings
#------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
	include $(SMING_HOME)/Windows.mk  
else
	UNAME := $(shell uname -s)
	include $(SMING_HOME)/$(UNAME).mk  
endif	

#==============================================================================
#   default serial settings
#------------------------------------------------------------------------------
#   use the maximum speed supported by your board to get things done faster
COM_SPEED_ESPTOOL   ?= 921600

#   use bootloader default (74880) to see bootloader messages
COM_SPEED_SERIAL    ?= 74880

DEFINES += COM_SPEED_SERIAL

#==============================================================================
#   path settings
#------------------------------------------------------------------------------
BUILD_BASE      = out/build
FW_BASE         = out/firmware

#   default values unless specified by the user
MODULES         ?= app                  
EXTRA_INCDIR    ?= include              
SPIFF_FILES     ?= files

#   SDK directories 
SDK_BASE        := $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS       := $(ESP_HOME)/utils/ESP8266
SDK_ROMS        := $(SDK_BASE)/bin
SDK_LIBDIR      := $(SDK_BASE)/lib
SDK_INCDIR      := $(SDK_BASE)/include

XTENSA_TOOLS    := $(ESP_HOME)/xtensa-lx106-elf/bin

USER_LIBDIR     := $(SMING_HOME)/compiler/lib/
THIRD_PARTY_DIR := $(SMING_HOME)/third-party

#==============================================================================
#   tools
#------------------------------------------------------------------------------
ESPTOOL2        := esptool2
SPIFFY          := $(SMING_HOME)/spiffy/spiffy

CC              := $(XTENSA_TOOLS)/xtensa-lx106-elf-gcc
CXX             := $(XTENSA_TOOLS)/xtensa-lx106-elf-g++
AR              := $(XTENSA_TOOLS)/xtensa-lx106-elf-ar
LD              := $(XTENSA_TOOLS)/xtensa-lx106-elf-gcc
OBJCOPY         := $(XTENSA_TOOLS)/xtensa-lx106-elf-objcopy
OBJDUMP         := $(XTENSA_TOOLS)/xtensa-lx106-elf-objdump
STRIP           := $(XTENSA_TOOLS)/xtensa-lx106-elf-strip

PERL            ?= perl
XXD             := xxd

ifeq ($(MANUAL_RESET),1)
	RESET_ESP       := $(PAUSE5) "*** PLEASE RESET ESP8266 NOW ***"
endif   

export COMPILE  := gcc
export PATH     := $(XTENSA_TOOLS):$(PATH)

#==============================================================================
#   makefile logging
#------------------------------------------------------------------------------
V ?= $(VERBOSE)
ifeq ("$(V)","1")
	Q       :=
	vecho   := @true
else
	Q       := @
	vecho   := @echo
endif

#==============================================================================
#   intermediate files
#------------------------------------------------------------------------------
#   name for the target project
TARGET          = app

APP_AR          := $(BUILD_BASE)/$(TARGET)_app.a
TARGET_OUT_0    := $(BUILD_BASE)/$(TARGET)_0.out
TARGET_OUT_1    := $(BUILD_BASE)/$(TARGET)_1.out

#==============================================================================
#   wifi settings
#------------------------------------------------------------------------------
#   we will use global WiFi settings from Eclipse Environment Variables, if possible
WIFI_SSID       ?= ""
WIFI_PWD        ?= ""

ifneq ($(WIFI_SSID), "")
	DEFINES += WIFI_SSID WIFI_PWD
endif

#==============================================================================
#   do flash address calculations
#------------------------------------------------------------------------------
hexcalc          = $(shell $(PERL) -e"printf '0x%06X', $(1)")

#   convert suffix 'K' and 'M' 
mem_size        := $(patsubst %M,(% * 1024 * 1024),$(SPI_SIZE))
mem_size        := $(patsubst %K,(% * 1024),$(mem_size))
MEM_SIZE        := $(call hexcalc, $(mem_size))

#   system parameter area 
ESP_PARM_ADDR   := $(call hexcalc, $(MEM_SIZE) - 0x2000)
ESP_INIT_ADDR   := $(call hexcalc, $(MEM_SIZE) - 0x4000)

#   rf calibration sector
RF_CAL_ADDR     := $(call hexcalc, $(ESP_INIT_ADDR) - 0x1000)

#   user flash area
USER_FLASH_SIZE ?= 0
USER_FLASH_ADDR := $(call hexcalc, $(RF_CAL_ADDR) - $(USER_FLASH_SIZE))

#   2nd spiffs start
SPIFF_SIZE      ?= 0
RBOOT_SPIFFS_1  ?= $(call hexcalc, $(USER_FLASH_ADDR) - $(SPIFF_SIZE))

#   1st spiffs start
RBOOT_SPIFFS_0  ?= $(call hexcalc, $(RBOOT_SPIFFS_1) - $(MEM_SIZE) / 2)

#   This value is only used to detect when the firmware is too large, will
#   otherwise crash on boot with "Rom 0 is bad". A safe value seems to be 
#   RBOOT_SPIFFS_0 - 0x8000 (found more or less by trial end error).
IROM0_SIZE      := $(call hexcalc, $(RBOOT_SPIFFS_0) - 0x8000)

#   org is always 0x40202010 for the 1st sming rom
IROM0_ORG0      := 0x40202010

#   this is where rboot expects the 2nd rom on small devices
IROM0_ORG1      := $(call hexcalc, $(IROM0_ORG0) + $(MEM_SIZE) / 2)

ifeq ($(V),1)
    $(info )
    $(info FLASH MEMORY MAP)
    $(info )
    $(info MEM_SIZE        = $(MEM_SIZE))
    $(info ESP_PARM_ADDR   = $(ESP_PARM_ADDR))
    $(info ESP_INIT_ADDR   = $(ESP_INIT_ADDR))
    $(info RF_CAL_ADDR     = $(RF_CAL_ADDR))
    $(info USER_FLASH_ADDR = $(USER_FLASH_ADDR) ($(USER_FLASH_SIZE) bytes))
    $(info RBOOT_SPIFFS_1  = $(RBOOT_SPIFFS_1) ($(SPIFF_SIZE) bytes))
    $(info RBOOT_SPIFFS_0  = $(RBOOT_SPIFFS_0) ($(SPIFF_SIZE) bytes))
    $(info )
endif

DEFINES += RF_CAL_ADDR USER_FLASH_ADDR USER_FLASH_SIZE RBOOT_SPIFFS_0 RBOOT_SPIFFS_1

#==============================================================================
#   rboot settings
#------------------------------------------------------------------------------
ifeq ($(SPI_SIZE),1M)
	#   two rom mode (no hardware mapping, different rom files required)
	RBOOT_TWO_ROMS  := 1
else    
	#   TODO: add support for more flash sizes here
	RBOOT_TWO_ROMS  := 0
endif

#   RBOOT_BIG_FLASH seems to be required even for 1M devices?
RBOOT_BIG_FLASH ?= 1

#   input linker files for 1st and 2nd rom
ROM0_LD         ?= $(BUILD_BASE)/rom0.ld
ROM1_LD         ?= $(BUILD_BASE)/rom1.ld

#   special rboot flags 
RBOOT_RTC_ENABLED   ?= 0
RBOOT_GPIO_ENABLED  ?= 0
RBOOT_INTEGRATION	:= 1

DEFINES += RBOOT_TWO_ROMS BOOT_BIG_FLASH RBOOT_RTC_ENABLED RBOOT_GPIO_ENABLED RBOOT_INTEGRATION
	
ifeq ($(SPIFF_SIZE),0)
	DISABLE_SPIFFS = 1
else    
	DISABLE_SPIFFS = 0
endif

DEFINES += SPIFF_SIZE DISABLE_SPIFFS

#==============================================================================
#   serial flash settings
#------------------------------------------------------------------------------
SPI_SPEED       ?= 40
SPI_MODE        ?= qio
SPI_SIZE_M      := $(shell $(PERL) -e"printf '%dm', $(MEM_SIZE) >> 17")     

ESPTOOL_FLAGS   := -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL)
ESPTOOL_IMGFLAGS:= -ff $(SPI_SPEED)m -fm $(SPI_MODE) -fs $(SPI_SIZE_M)

#==============================================================================
#   sming debug output
#------------------------------------------------------------------------------
#   let debugf print print filenames and line numbers
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

#   debug verbose level (DEBUG=3 INFO=2 WARNING=1 ERROR=0)
DEBUG_VERBOSE_LEVEL ?= 2

#==============================================================================
#   rom files
#------------------------------------------------------------------------------
#   boot loader rom
RBOOT_BIN       := $(FW_BASE)/rboot.bin

#   application roms
ROM0_BIN        := $(FW_BASE)/rom0.bin
ifeq ($(RBOOT_TWO_ROMS),1)
	ROM1_BIN        := $(FW_BASE)/rom1.bin
endif

#   spiffs rom
ifneq ($(SPIFF_SIZE),0)
	SPIFF_BIN       := $(FW_BASE)/spiff_rom.bin
endif

FLASH_FILES     := $(RBOOT_BIN) $(ROM0_BIN) $(SPIFF_BIN)
OTA_FILES       := $(ROM0_BIN) $(ROM1_BIN) $(SPIFF_BIN)

#==============================================================================
#   source, build and include directories
#------------------------------------------------------------------------------
MODULES         += $(THIRD_PARTY_DIR)/rboot/appcode

ifeq ($(ENABLE_GDB), 1)
	MODULES         += $(THIRD_PARTY_DIR)/gdbstub
endif

SRC_DIR         := $(MODULES)
BUILD_DIR       := $(addprefix $(BUILD_BASE)/,$(MODULES))

EXTRA_INCDIR    += $(SMING_HOME)/include $(SMING_HOME)/ $(SMING_HOME)/system/include 
EXTRA_INCDIR    += $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore 
EXTRA_INCDIR    += $(SMING_HOME)/Services/SpifFS 
EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/rboot $(THIRD_PARTY_DIR)/rboot/appcode 
EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/spiffs/src

#==============================================================================
#   clean-up and rebuild  
#------------------------------------------------------------------------------
.PHONY: all clean rebuild 

all: dirs $(CUSTOM_TARGETS) $(FLASH_FILES) $(OTA_FILES) 

clean:
	$(Q) rm -rf $(BUILD_BASE) $(FW_BASE) $(CUSTOM_TARGETS)

rebuild: clean all

#==============================================================================
#   create required directories
#------------------------------------------------------------------------------
.phony: dirs

$(BUILD_DIR) $(FW_BASE):
	$(Q) mkdir -p $@

dirs: $(BUILD_DIR) $(FW_BASE)

#==============================================================================
#   (re-)compile sming framework if required
#------------------------------------------------------------------------------
LIBSMING        := sming
SMING_FEATURES  := none
ifeq ($(ENABLE_SSL),1)
	LIBSMING        := smingssl
	SMING_FEATURES  := SSL
endif

LIBS        += $(LIBSMING)
NEEDED_A    += $(USER_LIBDIR)/lib$(LIBSMING).a

$(USER_LIBDIR)/lib$(LIBSMING).a:
	$(vecho) "(Re)compiling Sming. Enabled features: $(SMING_FEATURES). This may take some time"
	$(Q) $(MAKE) -C $(SMING_HOME) clean V=$(V) ENABLE_SSL=$(ENABLE_SSL) SMING_HOME=$(SMING_HOME)
	$(Q) $(MAKE) -C $(SMING_HOME) V=$(V) ENABLE_SSL=$(ENABLE_SSL) SMING_HOME=$(SMING_HOME)

#==============================================================================
#   we need to weaken one symbol in libmain for rboot big flash support
#------------------------------------------------------------------------------
LIBMAIN         = main
LIBMAIN_SRC     = $(SDK_LIBDIR)/libmain.a

ifeq ($(ENABLE_CUSTOM_HEAP),1)
	LIBMAIN         := mainmm
	LIBMAIN_SRC     := $(USER_LIBDIR)libmainmm.a
endif

ifeq ($(RBOOT_BIG_FLASH),1)
	LIBMAIN         := main2
	LIBMAIN_DST     := $(BUILD_BASE)/libmain2.a
	NEEDED_A        += $(LIBMAIN_DST)
endif

LIBS += $(LIBMAIN)

$(LIBMAIN_DST): $(LIBMAIN_SRC)
	$(vecho) "OC $@"
	@$(OBJCOPY) -W Cache_Read_Enable_New $^ $@

#==============================================================================
#   custom pwm support
#------------------------------------------------------------------------------
LIBPWM          := pwm
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	LIBPWM          := pwm_open
	LIBPWM_DST      := $(USER_LIBDIR)/lib$(LIBPWM).a
	NEEDED_A        += $(LIBPWM_DST)
endif

LIBS += $(LIBPWM) 

$(LIBPWM_DST): compiler/lib/libpwm_open.a
	$(Q) $(MAKE) -C $(SMING_HOME) compiler/lib/libpwm_open.a ENABLE_CUSTOM_PWM=1

#==============================================================================
#   SSL support using axTLS
#------------------------------------------------------------------------------
ifeq ($(ENABLE_SSL),1)
	LIBS            += axtls    
	EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/axtls-8266 $(THIRD_PARTY_DIR)/axtls-8266/ssl $(THIRD_PARTY_DIR)/axtls-8266/crypto 
	AXTLS_FLAGS     = -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1)  
		AXTLS_FLAGS     += -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1
	endif
	
	CFLAGS          += $(AXTLS_FLAGS)  
	CXXFLAGS        += $(AXTLS_FLAGS)   
	NEEDED_CC       += include/ssl/private_key.h
endif

CURRENT_DIR     := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

include/ssl/private_key.h:
	$(vecho) "Generating unique certificate and key. This may take some time"
	$(Q) mkdir -p $(CURRENT_DIR)/include/ssl/
	$(Q) AXDIR=$(CURRENT_DIR)/include/ssl/  $(THIRD_PARTY_DIR)/axtls-8266/tools/make_certs.sh 

#==============================================================================
#   custom lwip support
#------------------------------------------------------------------------------
ENABLE_CUSTOM_LWIP ?= 1

LIBLWIP         := lwip
ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	LIBLWIP         := lwip_open
	ifeq ($(ENABLE_ESPCONN), 1)
		LIBLWIP = lwip_full
	endif
	LIBLWIP_DST     := $(USER_LIBDIR)/lib$(LIBLWIP).a
	EXTRA_INCDIR    += $(SMING_HOME)/third-party/esp-open-lwip/include  
	NEEDED_A        += $(LIBLWIP_DST)
endif

LIBS += $(LIBLWIP) 

$(LIBLWIP_DST):
	$(Q) $(MAKE) -C $(SMING_HOME) compiler/lib/$(notdir $@) ENABLE_CUSTOM_LWIP=1 ENABLE_ESPCONN=$(ENABLE_ESPCONN)

#==============================================================================
#   compile our c and cpp source files
#------------------------------------------------------------------------------
CFLAGS          += -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -finline-functions -fdata-sections -ffunction-sections -D__ets__ -DICACHE_FLASH -DARDUINO=106 $(USER_CFLAGS)

ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS          += -Os -DSMING_RELEASE=1
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS          += -Og -ggdb -DGDBSTUB_FREERTOS=0 -DENABLE_GDB=1
	EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/gdbstub
	STRIP           := @true
else
	CFLAGS          += -Os -g
	STRIP           := @true
endif

#   Append debug options
CFLAGS          += -DCUST_FILE_BASE=$$(subst /,_,$(subst .,_,$$*)) -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)
CXXFLAGS        = $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

#   extra flags
CFLAGS          += $(foreach d,$(DEFINES),-D$d=$($d))

#	automatic dependency tracking 
#	s. a. http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
NEEDED_CC       += $1/%.d
CFLAGS          += -MT $$@ -MMD -MP -MF $1/temp.d
POSTCOMPILE     = mv -f $1/temp.d $1/$$*.d
DEP_FILES		= $(addsuffix /%.d, $(BUILD_DIR))
$(DEP_FILES): ;
.PRECIOUS: $(DEP_FILES)
include $(foreach bdir, $(BUILD_DIR), $(wildcard $(bdir)/*.d))

#   include directories
INCDIR          := $(addprefix -I,$(SRC_DIR))
MODULE_INCDIR   := $(addsuffix /include,$(INCDIR))
EXTRA_INCDIR    := $(addprefix -I,$(EXTRA_INCDIR))

#   find and process source files in all our source directories
define compile-objects
$1/%.o: %.c $(NEEDED_CC)
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) -I$(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@   
	$(POSTCOMPILE)
$1/%.o: %.cpp $(NEEDED_CC)
	$(vecho) "C+ $$<" 
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) -I$(SDK_INCDIR) $(CXXFLAGS) -c $$< -o $$@
	$(POSTCOMPILE)
endef

vpath %.c       $(SRC_DIR)
vpath %.cpp     $(SRC_DIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
	
#==============================================================================
#   create an archive from our object files
#------------------------------------------------------------------------------
#   build a list of all object files
SRC             := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
C_OBJ           := $(patsubst %.c,%.o,$(SRC))
CXX_OBJ         := $(patsubst %.cpp,%.o,$(C_OBJ))
OBJ             := $(patsubst %.o,$(BUILD_BASE)/%.o,$(CXX_OBJ))

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

#==============================================================================
#   create custom linker scripts
#------------------------------------------------------------------------------
$(ROM0_LD): $(SMING_HOME)/compiler/ld/rboot.rom0.ld
	$(PERL) -ple "s{(^\s*irom0_0_seg *: *).*}{\\1org = $(IROM0_ORG0), len = $(IROM0_SIZE)}" $< >$@

$(ROM1_LD): $(SMING_HOME)/compiler/ld/rboot.rom0.ld
	$(PERL) -ple "s{(^\s*irom0_0_seg *: *).*}{\\1org = $(IROM0_ORG1), len = $(IROM0_SIZE)}" $< >$@
	
#==============================================================================
#   link the main object file
#------------------------------------------------------------------------------
LIBS            += microc microgcc hal phy pp net80211 wpa crypto smartconfig $(EXTRA_LIBS)
LIBS            := $(addprefix -l,$(LIBS))
	
LDFLAGS         := -nostdlib -u call_user_start -u Cache_Read_Enable_New -Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map -Wl,-wrap,system_restart_local 
LDFLAGS         += $(addprefix -L,$(USER_LIBDIR) $(SDK_LIBDIR) $(BUILD_BASE) $(SMING_HOME)/compiler/ld)

$(TARGET_OUT_0): $(APP_AR) $(ROM0_LD) $(NEEDED_A) 
	$(vecho) "LD $@"
	$(Q) $(LD) -T$(ROM0_LD) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@
	$(Q) $(MEMANALYZER) $@ 
	$(Q) $(STRIP) $@

$(TARGET_OUT_1): $(APP_AR) $(ROM1_LD) $(NEEDED_A) 
	$(vecho) "LD $@"
	$(Q) $(LD) -T$(ROM1_LD) $(LDFLAGS) -Wl,--start-group $(APP_AR) $(LIBS) -Wl,--end-group -o $@
	$(Q) $(STRIP) $@

#==============================================================================
#   create application rom files 
#------------------------------------------------------------------------------
#   filenames and options for generating rBoot rom images with esptool2
RBOOT_E2_SECTS     ?= .text .data .rodata
RBOOT_E2_USER_ARGS ?= -quiet -bin -boot2

$(ROM0_BIN): $(TARGET_OUT_0)
	$(vecho) "ESPTOOL2 $@"
	@$(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $(TARGET_OUT_0) $@ $(RBOOT_E2_SECTS)

$(ROM1_BIN): $(TARGET_OUT_1)
	$(vecho) "ESPTOOL2 $@"
	@$(ESPTOOL2) $(RBOOT_E2_USER_ARGS) $(TARGET_OUT_1) $@ $(RBOOT_E2_SECTS)

#==============================================================================
#   create spiffs rom
#------------------------------------------------------------------------------
.PHONY: spiff_clean spiff_update

SPIFF_SRCS      := $(wildcard $(SPIFF_FILES)/*)

$(SPIFF_FILES):
	$(info create empty spiffs source directory $@ ...)
	mkdir -p $@

$(SPIFF_BIN): $(SPIFF_FILES) $(SPIFF_SRCS)
	$(Q) $(SPIFFY) $(SPIFF_SIZE) $(SPIFF_FILES) $@

spiff_clean: 
	$(info removing $(SPIFF_BIN) ...)
	$(Q) rm -rf $(SPIFF_BIN)
	
spiff_update: spiff_clean $(SPIFF_BIN)
	
#==============================================================================
#   create rboot bootloader rom
#------------------------------------------------------------------------------
export RBOOT_BIG_FLASH
export RBOOT_BUILD_BASE     := $(abspath $(BUILD_BASE))
export RBOOT_FW_BASE        := $(abspath $(FW_BASE))
export RBOOT_RTC_ENABLED
export RBOOT_GPIO_ENABLED
export RBOOT_ROM1_ADDR
export RBOOT_ROM2_ADDR
export SPI_SIZE
export SPI_MODE
export SPI_SPEED
export ESPTOOL2

$(RBOOT_BIN):
	$(MAKE) -C $(THIRD_PARTY_DIR)/rboot RBOOT_GPIO_ENABLED=$(RBOOT_GPIO_ENABLED)

#==============================================================================
#   initialize device
#------------------------------------------------------------------------------
.PHONY: flashinit

ESP_INIT_DATA   := $(SDK_ROMS)/esp_init_data_default.bin

#   special init data required for system_get_vdd33()
ifeq ($(GET_VDD33),1) 
	ESP_INIT_ROM := $(BUILD_BASE)/esp_init_data_vdd33.bin
else
	ESP_INIT_ROM := $(ESP_INIT_DATA)
endif

INIT_ROM_FLAGS   = $(RF_CAL_ADDR)   $(SDK_ROMS)/blank.bin 
INIT_ROM_FLAGS  += $(ESP_INIT_ADDR) $(ESP_INIT_ROM)
INIT_ROM_FLAGS  += $(ESP_PARM_ADDR) $(SDK_ROMS)/blank.bin

flashinit: kill_term $(ESP_INIT_ROM)
	$(info perform full flash erase and load system parameters)
	$(Q) $(ESPTOOL) $(ESPTOOL_FLAGS) erase_flash
	$(Q) $(RESET_ESP)
	$(Q) $(ESPTOOL) $(ESPTOOL_FLAGS) write_flash $(ESPTOOL_IMGFLAGS) $(INIT_ROM_FLAGS)

$(ESP_INIT_ROM): dirs $(ESP_INIT_DATA)
	$(info patch default init data for system_get_vdd33)
	$(Q) cp $< $<.tmp
	$(Q) echo "6B: FF" | $(XXD) -r - $<.tmp
	$(Q) mv $<.tmp $@

#==============================================================================
#   flash device
#------------------------------------------------------------------------------
.PHONY: flash

FLASH_ROM_FLAGS := 0x00000 $(RBOOT_BIN) 0x02000 $(ROM0_BIN)
ifneq ($(SPIFF_SIZE),0)
	FLASH_ROM_FLAGS += $(RBOOT_SPIFFS_0) $(SPIFF_BIN)
endif

flash: kill_term dirs $(CUSTOM_TARGETS) $(FLASH_FILES)
	$(Q) $(ESPTOOL) $(ESPTOOL_FLAGS) write_flash $(ESPTOOL_IMGFLAGS) $(FLASH_ROM_FLAGS)
	$(Q) $(TERMINAL)
	
#==============================================================================
#   manage terminal window
#------------------------------------------------------------------------------
.PHONY: terminal kill_term

terminal: 
	$(Q) $(TERMINAL)

kill_term:
	$(info Killing Terminal to free $(COM_PORT))
	-$(Q) $(KILL_TERM)
	