COMPONENT_NAME			:= DummySsl

COMPONENT_SRCDIRS := src
COMPONENT_INCDIRS := src

ifneq ($(SMING_ARCH),Host)
	EXTRA_LIBS := crypto
endif