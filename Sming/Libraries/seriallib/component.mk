COMPONENT_SUBMODULES	+= seriallib

SERIALLIB_ROOT := $(COMPONENT_PATH)/seriallib

COMPONENT_SRCDIRS := $(SERIALLIB_ROOT)/lib $(COMPONENT_PATH)/src
COMPONENT_INCDIRS := $(COMPONENT_SRCDIRS) $(COMPONENT_PATH)/include