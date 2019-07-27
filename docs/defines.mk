# Included before the component.mk file for a Component, Library or sample is parsed
#
# Define (or undefine) anything in here to get as much information as possible into the docs
#

# Ensure defaults get picked up for these
ENABLE_ESPCONN		:=
ENABLE_CUSTOM_LWIP	:=

# Enable optional Components
ENABLE_CUSTOM_HEAP	:= 1
ENABLE_SSL			:= 1
SDK_BASE			:= $(SMING_HOME)
