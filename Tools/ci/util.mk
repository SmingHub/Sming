#
# CI utilities
#

include $(SMING_HOME)/util.mk

SMING_HOME := $(patsubst %/,%,$(call FixPath,$(SMING_HOME)))
CI_TOOLS_DIR := $(abspath $(SMING_HOME)/../Tools/ci)

$(info CI_TOOLS_DIR = $(CI_TOOLS_DIR))

ifndef MODULE
MODULE := $(patsubst $(SMING_HOME)/%,%,$(shell pwd))
endif

export MODULE

# Send CI test framework notification
# $1 -> Name of test
# $2 -> Status (start, success, fail)
# $MODULE -> Test module name
define TestNotify
	$(CI_TOOLS_DIR)/testnotify.sh $1 $2
endef
