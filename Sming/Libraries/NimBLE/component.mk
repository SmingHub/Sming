COMPONENT_SUBMODULES := esp-nimble-cpp

COMPONENT_SOC := esp32 esp32c3 esp32s3

COMPONENT_SRCDIRS := esp-nimble-cpp/src
COMPONENT_INCDIRS := $(COMPONENT_SRCDIRS)

COMPONENT_CXXFLAGS := -Wno-unused-parameter
