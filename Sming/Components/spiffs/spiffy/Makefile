#
# Makefile for spiffy
#
# Pass in TARGET, BUILD_DIR , SPIFFS_SMING, SPIFFS_BASE
#

HOST_CC ?= gcc
HOST_LD ?= gcc

INCDIR := -I.. -I../spiffs/src
CFLAGS := -O2 -Wall -Wno-unused-value

ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

BUILD_DIR	?= .
TARGET		?= spiffy

OBJS := $(addprefix $(BUILD_DIR)/,spiffy.o spiffs_cache.o spiffs_nucleus.o spiffs_hydrogen.o spiffs_gc.o spiffs_check.o)

all: $(TARGET)

$(BUILD_DIR)/%.o: ../spiffs/src/%.c
	$(vecho) "CC $<"
	$(Q) $(HOST_CC) $(CFLAGS) $(INCDIR) -c $< -o $@

$(BUILD_DIR)/spiffy.o: spiffy.c
	$(vecho) "CC $<"
	$(Q) $(HOST_CC) $(CFLAGS) $(INCDIR) -c $< -o $@

$(TARGET): $(OBJS)
	$(vecho) "LD $@"
	$(Q) $(HOST_LD) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)
