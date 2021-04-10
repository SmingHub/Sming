## This sample depends on the HostEd component
COMPONENT_DEPENDS := Hosted
ENABLE_HOSTED :=

# If set the application should connect to a WIFI access point
# otherwise it will set its own access point
COMPONENT_VARS := CONNECT_TO_WIFI
CONNECT_TO_WIFI ?= 0

APP_CFLAGS = -DCONNECT_TO_WIFI=$(CONNECT_TO_WIFI)  
