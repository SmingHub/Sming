## Local build configuration
## Parameters configured here will override default and ENV values.

DEBUG_VERBOSE_LEVEL = 3

# Tell Sming about any Components we need
COMPONENT_DEPENDS := shared-test

# Don't need network
HOST_NETWORK_OPTIONS := --nonet

.PHONY: execute
execute: run
