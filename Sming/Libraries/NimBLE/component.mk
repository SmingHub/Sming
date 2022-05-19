COMPONENT_SUBMODULES := esp-nimble-cpp
# TODO: Add later support for the other Esp32 variants
COMPONENT_SOC := esp32

COMPONENT_DEPENDS := bluetooth 

COMPONENT_SRCDIRS := esp-nimble-cpp/src
COMPONENT_INCDIRS := $(COMPONENT_SRCDIRS)
