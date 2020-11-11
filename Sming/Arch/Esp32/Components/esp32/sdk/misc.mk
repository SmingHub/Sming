#
# Helper makefile invoked from component.mk
#

include $(SMING_HOME)/build.mk

all:

# List of all generated SDK libraries
SDK_ARCHIVE_DIRS = $(foreach c,$(SDK_COMPONENTS),$(SDK_BUILD_BASE)/esp-idf/$c $(call ListAllSubDirs,$(SDK_BUILD_BASE)/esp-idf/$c))
SDK_ARCHIVE_LIST = $(sort $(foreach d,$(SDK_ARCHIVE_DIRS),$(wildcard $d/*.a)))

#
# Need to suppress errors here as there are multiple copies of some libraries
#
.PHONY: copylibs
copylibs:
	$(info Copying generated SDK libraries)
	$(Q) cp -u $(SDK_ARCHIVE_LIST) $(SDK_COMPONENT_LIBDIR)/
