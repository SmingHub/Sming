#   sming.mk
#
#   to do:
#       - LTO?
#
#   done:
#       - roms with and without bootloader
#       - automatic flash address caclulation
#       - automatic linker script creation
#       - dependency tracking for spiffs sources
#       - dependency tracking for all header files included in cpp and c sources
#       - memanalyzer added
#       - automatic patch of esp_init_data_default.bin for system_get_vdd33()
#       - improved terminal handling for windows
#       - removed vpath to avoid source file collisions
#       - EXTRA_SRC option added
#       - more specific include paths to avoid header file collisions
#       - use Python for address calculations if available instead of Perl 
#       - use sed for linker file creation instead of Perl 
#
#   remarks:
#       - old variable names kept for compatibility, even if some of them seem
#         a bit inconsistent (e. g. SPIFF_SIZE, SPIFF_FILES)
#
#==============================================================================
#   user settings
#------------------------------------------------------------------------------
#   your flash memory size (required)
ifeq ("$(SPI_SIZE)","")
    $(error SPI_SIZE not set)
endif   

#   enable rboot (default: 0)
RBOOT_ENABLED   ?= 0

#   your source directories (default: app)
MODULES         ?= app                  

#   your extra include directies (default: none)
EXTRA_INCDIR    ?= 

#   your extra libraries (default: none)
EXTRA_LIBS      ?= 

#   your extra source files (default: none)
EXTRA_SRC       ?= 

#   your spiffs size (default: 0)
SPIFF_SIZE      ?= 0

#   your source directory for spiffs files (default: files)
SPIFF_FILES     ?= files

#   your reserved flash area for user data (default: 0)
USER_FLASH_SIZE ?= 0

#   patch init data for system_get_vdd33 (default: 0)
GET_VDD33       ?= 0

#==============================================================================
#   load os specific settings
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

#   SDK directories 
SDK_BASE        ?= $(ESP_HOME)/sdk
SDK_TOOLS       ?= $(SDK_BASE)/tools
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
SED             ?= sed
XXD             := xxd

ifeq ($(MANUAL_RESET),1)
	RESET_ESP       := $(PAUSE5) "*** PLEASE RESET ESP8266 NOW ***"
endif   

export COMPILE  := gcc
export PATH     := $(XTENSA_TOOLS):$(PATH)

ifeq ("$(PYTHON)","")
	hexcalc         = $(shell $(PERL) -e"printf '0x%06X', $(1)")
	deccalc         = $(shell $(PERL) -e"printf '%d', $(1)")
else
	hexcalc         = $(shell $(PYTHON) -c"print(format(int($(1)),'\#08x'))")
	deccalc         = $(shell $(PYTHON) -c"print(int($(1)))")
endif   

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
#   do flash address calculations
#------------------------------------------------------------------------------
#   convert suffix 'K' and 'M' to get the flash size in bytes
mem_size        := $(patsubst %M,(% * 1024 * 1024),$(SPI_SIZE))
mem_size        := $(patsubst %K,(% * 1024),$(mem_size))
MEM_SIZE        := $(call hexcalc, $(mem_size))

#   system parameter area, rf calibration sector and optional user flash area
ESP_PARM_ADDR   := $(call hexcalc, $(MEM_SIZE) - 0x2000)
ESP_INIT_ADDR   := $(call hexcalc, $(MEM_SIZE) - 0x4000)
RF_CAL_ADDR     := $(call hexcalc, $(ESP_INIT_ADDR) - 0x1000)
USER_FLASH_ADDR := $(call hexcalc, $(RF_CAL_ADDR) - $(USER_FLASH_SIZE))

DEFINES += RF_CAL_ADDR USER_FLASH_ADDR USER_FLASH_SIZE

#==============================================================================
#   rboot specific settings
#------------------------------------------------------------------------------
ifeq ($(RBOOT_ENABLED), 1)
	RBOOT_INTEGRATION   := 1

	#	provide some default settings for common memory layouts
	#   (RBOOT_BIG_FLASH seems to be required even for 1M devices)
	RBOOT_RTC_ENABLED   ?= 0
	RBOOT_GPIO_ENABLED  ?= 0
	RBOOT_BIG_FLASH     ?= 1

	ifeq ($(SPI_SIZE),1M)
		RBOOT_TWO_ROMS  	?= 1
	else	
		RBOOT_TWO_ROMS      ?= 0
	endif

	#   place upper spiffs below user flash area
	RBOOT_SPIFFS_1  ?= $(call hexcalc, $(USER_FLASH_ADDR) - $(SPIFF_SIZE))

	#   place lower spiffs (we loose a few sectors to simplify things)
	RBOOT_SPIFFS_0  ?= $(call hexcalc, $(RBOOT_SPIFFS_1) - $(MEM_SIZE) / 2)

	#   generate linker files (Sming needs 0x2010 instead of 0x2000!)
	ROM0_ADDR       := 0x002010
	IROM0_SIZE      ?= $(call hexcalc, 0x100000 - $(ROM0_ADDR))
	ROM0_LD         ?= $(BUILD_BASE)/rom0.ld

	#   rom files
	RBOOT_BIN       := $(FW_BASE)/rboot.bin
	ROM0_BIN        := $(FW_BASE)/rom0.bin

	#	special settings for two-rom mode
	ifeq ($(RBOOT_TWO_ROMS),1)
		ROM1_BIN        := $(FW_BASE)/rom1.bin
		ROM1_LD         ?= $(BUILD_BASE)/rom1.ld
		
		#   this is where rboot expects the 2nd rom in two-rom mode
		ROM1_ADDR       ?= $(call hexcalc, $(ROM0_ADDR) + $(MEM_SIZE) / 2)

		#	this seems to be a save value (found by trial and error)
		IROM0_SIZE      := $(call hexcalc, $(RBOOT_SPIFFS_0) - 0x8000)
	endif

	OTA_FILES       += $(ROM0_BIN) $(ROM1_BIN)
	FLASH_FILES     += $(RBOOT_BIN) $(ROM0_BIN) 
	FLASH_ROM_FLAGS += 0x000000 $(RBOOT_BIN) 0x002000 $(ROM0_BIN)

	#   find rboot source files
	MODULES         += $(THIRD_PARTY_DIR)/rboot/appcode

	DEFINES += RBOOT_INTEGRATION  RBOOT_RTC_ENABLED RBOOT_GPIO_ENABLED 
	DEFINES += RBOOT_TWO_ROMS BOOT_BIG_FLASH RBOOT_SPIFFS_0 RBOOT_SPIFFS_1

#==============================================================================
#   non-rboot settings ("standalone applications")
#------------------------------------------------------------------------------
else
	#   place spiffs below user flash area
	RBOOT_SPIFFS_0  ?= $(call hexcalc, $(USER_FLASH_ADDR) - $(SPIFF_SIZE))
	
	#	generate linker files
	ROM0_ADDR       ?= 0x00A000
	IROM0_SIZE      ?= $(call hexcalc, 0x100000 - $(ROM0_ADDR))
	ROM0_LD         ?= $(BUILD_BASE)/rom0.ld

	#	rom files
	IMAGE_MAIN      := $(FW_BASE)/main.bin
	IMAGE_SDK       := $(FW_BASE)/sdk.bin
	FLASH_FILES     += $(IMAGE_MAIN) $(IMAGE_SDK) 
	FLASH_ROM_FLAGS += 0x000000 $(IMAGE_MAIN) $(ROM0_ADDR) $(IMAGE_SDK)
endif

#==============================================================================
#   show memory map
#------------------------------------------------------------------------------
ifeq ($(V),1)
    $(info )
    $(info FLASH MEMORY MAP)
    $(info )
    $(info MEM_SIZE        = $(MEM_SIZE))
    $(info ESP_PARM_ADDR   = $(ESP_PARM_ADDR))
    $(info ESP_INIT_ADDR   = $(ESP_INIT_ADDR))
    $(info RF_CAL_ADDR     = $(RF_CAL_ADDR))
    ifneq ($(USER_FLASH_SIZE),0)
        $(info USER_FLASH_ADDR = $(USER_FLASH_ADDR) ($(USER_FLASH_SIZE) bytes))
    endif
    ifneq ($(SPIFF_SIZE),0)
        $(info RBOOT_SPIFFS_0  = $(RBOOT_SPIFFS_0) ($(SPIFF_SIZE) bytes))
    endif
    $(info ROM0_ADDR       = $(ROM0_ADDR) ($(IROM0_SIZE) bytes))
    $(info )
endif

#==============================================================================
#   spiffs specific settings
#------------------------------------------------------------------------------
ifneq ($(SPIFF_SIZE),0)
	DISABLE_SPIFFS  := 0
	SPIFF_BIN       := $(FW_BASE)/spiff_rom.bin
	OTA_FILES       += $(SPIFF_BIN)
	FLASH_FILES     += $(SPIFF_BIN)
	FLASH_ROM_FLAGS += $(RBOOT_SPIFFS_0) $(SPIFF_BIN)

#------------------------------------------------------------------------------
else    
	DISABLE_SPIFFS  := 1

endif

DEFINES += SPIFF_SIZE DISABLE_SPIFFS  

#==============================================================================
#   serial flash settings
#------------------------------------------------------------------------------
SPI_SPEED       ?= 40
SPI_MODE        ?= qio
SPI_SIZE_M      := $(call deccalc, $(MEM_SIZE) >> 17)m    

ESPTOOL_FLAGS   := -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL)
ESPTOOL_IMGFLAGS:= -ff $(SPI_SPEED)m -fm $(SPI_MODE) -fs $(SPI_SIZE_M)

#==============================================================================
#   source, build and include directories
#------------------------------------------------------------------------------
ifeq ($(ENABLE_GDB), 1)
	MODULES         += $(THIRD_PARTY_DIR)/gdbstub
endif

SRC_DIR         := $(MODULES) $(patsubst %/,%,$(sort $(dir $(EXTRA_SRC))))
BUILD_DIR       := $(addprefix $(BUILD_BASE)/,$(SRC_DIR))

EXTRA_INCDIR    += $(SMING_HOME)/include $(SMING_HOME)/ $(SMING_HOME)/system/include 
EXTRA_INCDIR    += $(SMING_HOME)/Wiring $(SMING_HOME)/Libraries $(SMING_HOME)/SmingCore 
EXTRA_INCDIR    += $(SMING_HOME)/Services/SpifFS 
EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/spiffs/src
EXTRA_INCDIR    += $(THIRD_PARTY_DIR)/rboot $(THIRD_PARTY_DIR)/rboot/appcode 

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
#   use global wifi settings from eclipse environment variables, if possible
WIFI_SSID       ?= ""
WIFI_PWD        ?= ""

ifneq ($(WIFI_SSID), "")
	DEFINES += WIFI_SSID WIFI_PWD
endif

#==============================================================================
#   sming debug output
#------------------------------------------------------------------------------
#   let debugf print print filenames and line numbers
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

#   debug verbose level (DEBUG=3 INFO=2 WARNING=1 ERROR=0)
DEBUG_VERBOSE_LEVEL ?= 2

#==============================================================================
#   clean-up and rebuild  
#------------------------------------------------------------------------------
.PHONY: all clean rebuild 

all: dirs $(CUSTOM_TARGETS) $(FLASH_FILES) $(OTA_FILES) 

clean:
	$(Q) rm -rf $(BUILD_BASE) $(FW_BASE) $(CUSTOM_TARGETS)

rebuild: clean all

#   disable build-in rules
.SUFFIXES:      

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

#   extra include directories
CFLAGS          += $(addprefix -I,$(EXTRA_INCDIR)) -I$(SDK_INCDIR)

#   Append debug options
CFLAGS          += -DCUST_FILE_BASE=$(subst /,_,$(subst .,_,$*)) -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)
CXXFLAGS        = $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors

#   extra flags
CFLAGS          += $(foreach d,$(DEFINES),-D$d=$($d))

#   automatic dependency tracking 
#   s. a. http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

CFLAGS          += -MT $$@ -MMD -MP -MF $2/temp.d
POSTCOMPILE     = $(Q) mv -f $2/temp.d $2/$$*.d
DEP_FILES       = $(addsuffix /%.d, $(BUILD_DIR))
$(DEP_FILES): ;
.PRECIOUS: $(DEP_FILES)
include $(foreach bdir, $(BUILD_DIR), $(wildcard $(bdir)/*.d))

#   define compiler rules for all sources directories
define compile-source
$2/%.o: $1/%.cpp $2/%.d $(NEEDED_CC)
	$(vecho) "C+ $$<" 
	$(Q) $(CXX) -I$2 -I$2/include $(CXXFLAGS) -c $$< -o $$@
	$(POSTCOMPILE)

$2/%.o: $1/%.c $2/%.d $(NEEDED_CC)
	$(vecho) "CC $$<"
	$(Q) $(CC) -I$2 -I$2/include $(CFLAGS) -c $$< -o $$@   
	$(POSTCOMPILE)
endef

$(foreach dir,$(SRC_DIR),$(eval $(call compile-source,$(dir),$(addprefix $(BUILD_BASE)/,$(dir)))))
	
#==============================================================================
#   create an archive from our object files
#------------------------------------------------------------------------------
#   build list of all source and object files
SRC             := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
OBJ             := $(addprefix $(BUILD_BASE)/,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRC))))

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

#==============================================================================
#   create custom linker scripts
#------------------------------------------------------------------------------
IROM0_ORG0      := $(call hexcalc, 0x40200000 + $(ROM0_ADDR)) 
ROM1_ADDR		?= $(ROM0_ADDR)
IROM0_ORG1      := $(call hexcalc, 0x40200000 + $(ROM1_ADDR)) 

$(ROM0_LD): $(SMING_HOME)/compiler/ld/rboot.rom0.ld
	$(SED) -r "s/(^\s*irom0_0_seg *: *).*/\\1org = $(IROM0_ORG0), len = $(IROM0_SIZE)/" $< >$@

$(ROM1_LD): $(SMING_HOME)/compiler/ld/rboot.rom0.ld
	$(SED) -r "s/(^\s*irom0_0_seg *: *).*/\\1org = $(IROM0_ORG1), len = $(IROM0_SIZE)/" $< >$@
	
#==============================================================================
#   link the main object file
#------------------------------------------------------------------------------
LIBS            += microc microgcc hal phy pp net80211 wpa crypto smartconfig $(EXTRA_LIBS)
LIBS            := $(addprefix -l,$(LIBS))
	
LDFLAGS         = -nostdlib -u call_user_start -u Cache_Read_Enable_New -Wl,-static -Wl,--gc-sections -Wl,-Map=$(basename $@).map -Wl,-wrap,system_restart_local 
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
E2_FLAGS			+= -quiet
E2_SECTS_MAIN     	+= .text .data .rodata

#	rboot roms
$(ROM0_BIN): $(TARGET_OUT_0)
	$(vecho) "ESPTOOL2 $@"
	$(Q) $(ESPTOOL2) $(E2_FLAGS) -bin -boot2 $(TARGET_OUT_0) $@ $(E2_SECTS_MAIN)

$(ROM1_BIN): $(TARGET_OUT_1)
	$(vecho) "ESPTOOL2 $@"
	$(Q) $(ESPTOOL2) $(E2_FLAGS) -bin -boot2 $(TARGET_OUT_1) $@ $(E2_SECTS_MAIN)

#	non-rboot roms
$(IMAGE_MAIN): $(TARGET_OUT_0)
	$(vecho) "ESPTOOL2 $@"
	$(Q) $(ESPTOOL2) $(E2_FLAGS) -bin -boot0 $(TARGET_OUT_0) $@ $(E2_SECTS_MAIN)

$(IMAGE_SDK): $(TARGET_OUT_0)
	$(vecho) "ESPTOOL2 $@"
	$(Q) $(ESPTOOL2) $(E2_FLAGS) -lib $(TARGET_OUT_0) $@ 
	
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

$(RBOOT_BIN): $(NEEDED_A)
	$(MAKE) -C $(THIRD_PARTY_DIR)/rboot RBOOT_GPIO_ENABLED=$(RBOOT_GPIO_ENABLED)

#==============================================================================
#   initialize device
#------------------------------------------------------------------------------
.PHONY: flashinit

ESP_INIT_DATA   := $(SDK_ROMS)/esp_init_data_default.bin

#   special init data is required for system_get_vdd33()
ifeq ($(GET_VDD33),1) 
	ESP_INIT_ROM := $(BUILD_BASE)/esp_init_data_vdd33.bin
else
	ESP_INIT_ROM := $(ESP_INIT_DATA)
endif

INIT_ROM_FLAGS   = $(RF_CAL_ADDR)   $(SDK_ROMS)/blank.bin 
INIT_ROM_FLAGS  += $(ESP_INIT_ADDR) $(ESP_INIT_ROM)
INIT_ROM_FLAGS  += $(ESP_PARM_ADDR) $(SDK_ROMS)/blank.bin

flashinit: kill_term dirs $(ESP_INIT_ROM)
	$(info perform full flash erase and load system parameters)
	$(Q) $(ESPTOOL) $(ESPTOOL_FLAGS) erase_flash
	$(Q) $(RESET_ESP)
	$(Q) $(ESPTOOL) $(ESPTOOL_FLAGS) write_flash $(ESPTOOL_IMGFLAGS) $(INIT_ROM_FLAGS)

$(BUILD_BASE)/esp_init_data_vdd33.bin: $(ESP_INIT_DATA)
	$(info patch default init data for system_get_vdd33)
	$(Q) cp $< $<.tmp
	$(Q) echo "6B: FF" | $(XXD) -r - $<.tmp
	$(Q) mv $<.tmp $@

#==============================================================================
#   flash device
#------------------------------------------------------------------------------
.PHONY: flash

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
	