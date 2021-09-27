#
# Helper makefile invoked from component.mk
#

include $(SMING_HOME)/build.mk

all:

# List of all generated SDK libraries
SDK_ARCHIVE_DIRS = $(wildcard $(SDK_BUILD_BASE)/esp-idf/*)
SDK_ARCHIVE_LIST = $(sort $(foreach d,$(SDK_ARCHIVE_DIRS),$(wildcard $d/*.a)))

.PHONY: copylibs
copylibs:
	$(info Copying generated SDK libraries)
	$(Q) cp $(SDK_ARCHIVE_LIST) $(SDK_COMPONENT_LIBDIR)/
