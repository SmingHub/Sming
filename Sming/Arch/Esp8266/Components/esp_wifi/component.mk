COMPONENT_LIBNAME :=

# => WPS
COMPONENT_VARS		:= ENABLE_WPS
ifeq ($(ENABLE_WPS), 1)
   GLOBAL_CFLAGS	+= -DENABLE_WPS=1
   EXTRA_LIBS		:= wps
endif

EXTRA_LIBS := \
	phy \
	pp \
	net80211 \
	wpa \
	crypto \
	smartconfig
