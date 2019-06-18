
#
MODULES			+= $(ARCH_COMPONENTS)/esp_wifi
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_wifi/include


# APP

# => WPS
CONFIG_VARS	+= ENABLE_WPS
ifeq ($(ENABLE_WPS),1)
   CFLAGS	+= -DENABLE_WPS=1
   LIBS		+= wps
endif


EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_wifi/include

# => WPS
CONFIG_VARS += ENABLE_WPS
ifeq ($(ENABLE_WPS), 1)
   CFLAGS += -DENABLE_WPS=1
endif

