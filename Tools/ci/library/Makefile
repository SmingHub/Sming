# Don't bother with implicit checks
.SUFFIXES:

.NOTPARALLEL:

ifeq (,$(SMING_HOME))
$(error SMING_HOME not set)
endif

ifeq (,$(SMING_ARCH))
$(error SMING_ARCH not set)
endif

include $(SMING_HOME)/util.mk

APP_NAMES := $(patsubst $(CURDIR)/%,%,$(call ListSubDirs,samples) $(wildcard test))

ARCH_SOC := $(ARCH_$(SMING_ARCH)_SOC)

all: $(ARCH_SOC)

.PHONY: $(ARCH_SOC)
$(ARCH_SOC):
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
