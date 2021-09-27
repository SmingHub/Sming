## This sample depends on the HostEd component
COMPONENT_DEPENDS := Hosted
ENABLE_HOSTED :=

## And does not require WIFI
DISABLE_WIFI := 1

ENABLE_HOST_UARTID := 0
HOST_NETWORK_OPTIONS := --pause