#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
# Multi-architecture support by mikee47 <mike@sillyhouse.net>
#
#############################################################

.NOTPARALLEL:

# Make sure we're not pointing into a different Sming repo
override SMING_HOME := $(CURDIR)

include build.mk

.PHONY: all
all:
	$(info Please run `make` from your project directory!)

#
DOXYGEN := $(shell command -v doxygen 2> /dev/null)


##@Cleaning

.PHONY: dist-clean
dist-clean: submodules-clean samples-clean docs-clean ##Clean everything for all arch/build types
	-$(Q) rm -rf out

.PHONY: clean
clean: ##Remove all intermediate build files, but leave generated libraries and tools
	-$(Q) rm -rf $(BUILD_BASE)

.PHONY: components-clean
components-clean: ##Remove generated Component (user) libraries
	@echo Cleaning all Component libraries
	-$(Q) rm -f $(USER_LIBDIR)/*
	-$(Q) rm -rf $(OUT_BASE)


ALL_SUBMODULES = $(ListSubmodules)

.PHONY: submodules-clean
submodules-clean: ##Reset state of all third-party submodules
	@echo Cleaning all submodules...
	-$(Q) rm -rf $(ALL_SUBMODULES)
	-$(Q) rm -rf $(addprefix $(BUILD_BASE),$(ALL_SUBMODULES))
	$(Q) $(GIT) checkout $(ALL_SUBMODULES)


.PHONY: docs-clean
docs-clean: ##Clean the Sming documentation
	$(Q) $(MAKE) -C ../docs clean


##@Building

# Pull in all submodules, regardless of whether they're used or not
.PHONY: submodules
submodules: $(ALL_SUBMODULES:=/.submodule) ##Fetch all third-party submodules (but do not build)

.PHONY: test
test: Basic_Blink Basic_rBoot Basic_Ssl Basic_HwPWM ##Build a few sample applications

.PHONY: docs
docs: submodules ##Build the Sming documentation
	$(Q) $(MAKE) -C ../docs html


# For integration testing both samples and tests are moved outside of the repo.
SMING_PROJECTS_DIR	?= $(abspath $(SMING_HOME)/..)
SAMPLES_DIR			:= $(call FixPath,$(SMING_PROJECTS_DIR)/samples)
TESTS_DIR			:= $(call FixPath,$(SMING_PROJECTS_DIR)/tests)

SAMPLE_NAMES		= $(shell ls -1 $(SAMPLES_DIR))

.PHONY: samples $(SAMPLE_NAMES)
samples: | $(SAMPLE_NAMES) ##Build all sample applications

$(SAMPLE_NAMES):
	$(Q) $(MAKE) --no-print-directory -C $(SAMPLES_DIR)/$@


TESTS_COMPLETED = $(addsuffix /.complete,$(call ListSubDirs,$(TESTS_DIR)))
PHONY: tests
tests: $(TESTS_COMPLETED) ##Build and run all test applications

$(TESTS_COMPLETED):
	$(Q) $(MAKE) -C $(@D) execute
	$(Q) touch $@

##@Cleaning

.PHONY: samples-clean
samples-clean: ##Clean all sample applications (all arch/build types)
	@echo Cleaning all samples...
	-$(Q) cd $(SAMPLES_DIR) && rm -rf $(addsuffix /out,$(SAMPLE_NAMES))

CLEAN_TESTS := $(TESTS_COMPLETED:complete=clean)
.PHONY: tests-clean
tests-clean: $(CLEAN_TESTS) ##Clean all test applications (all arch/build types)
	-$(Q) rm -f $(TESTS_COMPLETED)

.PHONY: $(CLEAN_TESTS)
$(CLEAN_TESTS):
	@echo Cleaning '$(@D)'
	$(Q) $(MAKE) -C $(@D) clean


##@Developer

# Recursive wildcard search
# $1 -> list of directories
# $2 -> file extensions filters (using % as wildcard)
define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $2,$d))
endef

# Files that should follow our coding standards
CS_DIRS		= Core Platform Services Arch/*/Core Arch/*/Platform $(SAMPLES_DIR) $(TESTS_DIR)
CS_FILES	= $(call rwildcard,$(CS_DIRS:=/*),%.cpp %.h %.c)

.PHONY: cs
cs: ##Apply coding style to all core files
	$(if $(V),$(info Applying coding style to $(words $(CS_FILES)) files ...))
	@for FILE in $(CS_FILES); do \
		clang-format -i -style=file $$FILE; \
	done


##@Help

.PHONY: help
help: ##Show this help summary
	$(PrintHelp)
