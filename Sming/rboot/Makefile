#
# Makefile for rBoot
# https://github.com/raburton/esp8266
#

ESPTOOL2 ?= ../esptool2/esptool2

RBOOT_BUILD_BASE ?= build
RBOOT_FW_BASE    ?= firmware

ifndef XTENSA_BINDIR
CC := xtensa-lx106-elf-gcc
LD := xtensa-lx106-elf-gcc
else
CC := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
LD := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
endif

CFLAGS    = -Os -O3 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS   = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static
LD_SCRIPT = eagle.app.v6.ld

E2_OPTS = -quiet -bin -boot0

ifeq ($(RBOOT_BIG_FLASH),1)
	CFLAGS += -DBOOT_BIG_FLASH
endif
ifeq ($(SPI_SIZE), 256K)
	E2_OPTS += -256
else ifeq ($(SPI_SIZE), 512K)
	E2_OPTS += -512
else ifeq ($(SPI_SIZE), 1M)
	E2_OPTS += -1024
else ifeq ($(SPI_SIZE), 2M)
	E2_OPTS += -2048
else ifeq ($(SPI_SIZE), 4M)
	E2_OPTS += -4096
endif

.SECONDARY:

#all: $(RBOOT_BUILD_BASE) $(RBOOT_FW_BASE) $(RBOOT_FW_BASE)/rboot.bin $(RBOOT_FW_BASE)/testload1.bin $(RBOOT_FW_BASE)/testload2.bin
all: $(RBOOT_BUILD_BASE) $(RBOOT_FW_BASE) $(RBOOT_FW_BASE)/rboot.bin

$(RBOOT_BUILD_BASE):
	mkdir -p $@

$(RBOOT_FW_BASE):
	mkdir -p $@

$(RBOOT_BUILD_BASE)/rboot-stage2a.o: rboot-stage2a.c rboot-private.h rboot.h
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@
	
$(RBOOT_BUILD_BASE)/rboot-stage2a.elf: $(RBOOT_BUILD_BASE)/rboot-stage2a.o
	@echo "LD $@"
	@$(LD) -Trboot-stage2a.ld $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -o $@

$(RBOOT_BUILD_BASE)/rboot-hex2a.h: $(RBOOT_BUILD_BASE)/rboot-stage2a.elf
	@echo "FW $@"
	@$(ESPTOOL2) -quiet -header $< $@ .text

$(RBOOT_BUILD_BASE)/rboot.o: rboot.c rboot-private.h rboot.h $(RBOOT_BUILD_BASE)/rboot-hex2a.h
	@echo "CC $<"
	@$(CC) $(CFLAGS) -I$(RBOOT_BUILD_BASE) -c $< -o $@

$(RBOOT_BUILD_BASE)/%.o: %.c %.h
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(RBOOT_BUILD_BASE)/%.elf: $(RBOOT_BUILD_BASE)/%.o
	@echo "LD $@"
	@$(LD) -T$(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -o $@

$(RBOOT_FW_BASE)/%.bin: $(RBOOT_BUILD_BASE)/%.elf
	@echo "FW $@"
	@$(ESPTOOL2) $(E2_OPTS) $< $@ .text .rodata

clean:
	@echo "RM $(RBOOT_BUILD_BASE) $(RBOOT_FW_BASE)"
	@rm -rf $(RBOOT_BUILD_BASE)
	@rm -rf $(RBOOT_FW_BASE)
