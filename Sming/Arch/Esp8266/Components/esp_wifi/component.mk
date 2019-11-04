COMPONENT_LIBNAME :=

COMPONENT_DEPENDS := esp8266

EXTRA_LIBS := \
	phy \
	pp \
	net80211 \
	wpa \
	crypto

ifeq ($(ENABLE_WPS), 1)
	ENABLE_ESPCONN		:= 1
	EXTRA_LIBS			+= wps
endif

ifeq ($(ENABLE_SMART_CONFIG),1)
	ENABLE_ESPCONN		:= 1
	EXTRA_LIBS			+= smartconfig
endif
