#
# Build/test a library for either:
#
#    One SoC: Define SMING_SOC
#    All SoCs for an architecture: Define SMING_ARCH
#    All available SoCs: Neither SMING_SOC nor SMING_ARCH defined
#
#

.SUFFIXES:
.NOTPARALLEL:

ifeq (,$(SMING_HOME))
$(error SMING_HOME not set)
endif

include $(SMING_HOME)/util.mk

APP_NAMES := $(patsubst $(CURDIR)/%,%,$(call ListSubDirs,samples) $(wildcard test))

ifneq (,$(SMING_SOC))
all: $(SMING_SOC)
else ifneq (,$(SMING_ARCH))
all: $(ARCH_$(SMING_ARCH)_SOC)
else
all: $(AVAILABLE_SOCS)
endif

.PHONY: $(AVAILABLE_SOCS)
$(AVAILABLE_SOCS):
	$(Q) $(MAKE) --no-print-directory -f $(firstword $(MAKEFILE_LIST)) SMING_SOC=$@ STRICT=1 apps run-test

.PHONY: apps
apps: $(APP_NAMES)

.PHONY: $(APP_NAMES)
$(APP_NAMES):
	@printf "\n\n** Building $@ for $(SMING_SOC) **\n\n"
	$(Q) $(MAKE) --no-print-directory -C $@ PIP_ARGS=-q python-requirements sample

.PHONY: run-test
run-test:
ifeq (,$(wildcard test))
	@printf "\n\n** No test directory found - skipping **\n\n"
else ifeq ($(SMING_ARCH),Host)
	$(Q) $(MAKE) --no-print-directory -C test execute
endif

clean:
	$(Q) rm -rf $(addsuffix /out,$(APP_NAMES))
