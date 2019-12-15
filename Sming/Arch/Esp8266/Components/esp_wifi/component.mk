COMPONENT_LIBNAME :=

COMPONENT_DEPENDS := esp8266

EXTRA_LIBS := \
	pp \
	net80211 \
	wpa

ifeq ($(ENABLE_WPS), 1)
	ENABLE_ESPCONN		:= 1
	EXTRA_LIBS			+= wps
endif

ifeq ($(ENABLE_SMART_CONFIG),1)
	ENABLE_ESPCONN		:= 1
	EXTRA_LIBS			+= smartconfig
endif
