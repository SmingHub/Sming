COMPONENT_ARCH := $(SMING_ARCH)
ifeq ($(COMPONENT_ARCH),Host)
	COMPONENT_ARCH := Esp8266
endif

COMPONENT_SRCDIRS := \
        src \
        src/Arch/$(COMPONENT_ARCH)

COMPONENT_INCDIRS := \
        src/include \
        src/Arch/$(COMPONENT_ARCH)/include

ifeq ($(COMPONENT_ARCH),Esp8266)
	COMPONENT_DEPENDS += rboot
endif