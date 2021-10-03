#
# freertos
#

ifdef CONFIG_FREERTOS_DEBUG_OCDAWARE
EXTRA_LDFLAGS += -Wl,--undefined=uxTopUsedPriority
endif
