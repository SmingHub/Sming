DISABLE_SPIFFS = 1
DEBUG_VERBOSE_LEVEL = 2
SPI_SIZE = 4M

ARDUINO_LIBRARIES := \
	SmingTest \
	ArduinoJson5 \
	ArduinoJson6

COMPONENT_DEPENDS := malloc_count

# Don't need network
HOST_NETWORK_OPTIONS := --nonet

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
