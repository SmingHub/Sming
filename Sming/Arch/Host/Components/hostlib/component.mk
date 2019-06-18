
MODULES			+= $(ARCH_COMPONENTS)/hostlib

LIBS += pthread

ifeq ($(UNAME),Windows)
LIBS += wsock32
endif

