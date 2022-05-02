## This sample depends on the HostEd component
COMPONENT_DEPENDS := Hosted
ENABLE_HOSTED :=

## And does not require networking
DISABLE_NETWORK := 1

ENABLE_HOST_UARTID := 0
HOST_NETWORK_OPTIONS := --pause

COMPONENT_CXXFLAGS += \
	-Wno-pmf-conversions
