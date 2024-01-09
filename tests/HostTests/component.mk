ifeq ($(SMING_ARCH),Rp2040)
HWCONFIG = host-tests-1m
else ifeq ($(SMING_ARCH),Esp32)
HWCONFIG = host-tests-esp32
else
HWCONFIG = host-tests
endif

DEBUG_VERBOSE_LEVEL = 2

COMPONENT_INCDIRS := include
COMPONENT_SRCDIRS := \
	app \
	modules \
	Arch/$(SMING_ARCH)

ARDUINO_LIBRARIES := \
	SmingTest \
	ArduinoJson5 \
	ArduinoJson6

ifeq ($(SMING_ARCH),Host)
	ARDUINO_LIBRARIES += Hosted
endif

COMPONENT_DEPENDS := \
	malloc_count

ifneq ($(DISABLE_NETWORK),1)
COMPONENT_SRCDIRS += \
	modules/Network \
	modules/Network/Arch/$(SMING_ARCH)
COMPONENT_DEPENDS += \
	axtls-8266 \
	bearssl-esp8266
endif

ifeq ($(UNAME),Windows)
# Network tests run on Linux only
HOST_NETWORK_OPTIONS := --nonet
endif

# Time in milliseconds to pause after a test group has completed
CONFIG_VARS += TEST_GROUP_INTERVAL
TEST_GROUP_INTERVAL ?= 500
APP_CFLAGS += -DTEST_GROUP_INTERVAL=$(TEST_GROUP_INTERVAL)

# Time in milliseconds to wait before re-starting all tests
# Set to 0 to perform a system restart after all tests have completed
CONFIG_VARS += RESTART_DELAY
ifndef RESTART_DELAY
ifeq ($(SMING_ARCH),Host)
RESTART_DELAY = 0
else
RESTART_DELAY ?= 10000
endif
endif
APP_CFLAGS += -DRESTART_DELAY=$(RESTART_DELAY)

.PHONY: execute
execute: flash run

SPIFFSGEN_BIN := out/spiff_rom_test.bin
CUSTOM_TARGETS += $(SPIFFSGEN_BIN)
$(SPIFFSGEN_BIN):
	$(Q) $(SPIFFSGEN_SMING) 0x10000 spiffsgen/build $@

clean: resource-clean
.PHONY: resource-clean
resource-clean:
	$(Q) rm -f $(SPIFFSGEN_BIN)
