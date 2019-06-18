
# => SMING

MODULES			+= $(ARCH_COMPONENTS)/esp_wifi
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_wifi/include

# => APP

EXTRA_INCDIR += $(ARCH_COMPONENTS)/esp_wifi/include

# => SMING

CONFIG_VARS += ENABLE_WPS
ifeq ($(ENABLE_WPS), 1)
   CFLAGS += -DENABLE_WPS=1
endif

# => APP

CONFIG_VARS	+= ENABLE_WPS
ifeq ($(ENABLE_WPS),1)
   CFLAGS	+= -DENABLE_WPS=1
   LIBS		+= wps
endif

