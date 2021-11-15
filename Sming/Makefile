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


##@Cleaning

.PHONY: dist-clean
dist-clean: submodules-clean samples-clean docs-clean tests-clean ##Clean everything for all arch/build types
	-$(Q) rm -rf out

.PHONY: clean
clean: ##Remove all intermediate build files, but leave generated libraries and tools
	-$(Q) rm -rf $(BUILD_BASE)

.PHONY: components-clean
components-clean: ##Remove generated Component (user) libraries
	@echo Cleaning all Component libraries
	-$(Q) rm -f $(USER_LIBDIR)/*
	-$(Q) rm -rf $(OUT_BASE)

# Build list of relative paths to all submodules
ALL_SUBMODULES := $(patsubst $(SMING_HOME)/%,%,$(ListSubmodules))


.PHONY: submodules-clean
submodules-clean: ##Reset state of all third-party submodules
	@echo Cleaning all submodules...
	-$(Q) rm -rf $(ALL_SUBMODULES)
	-$(Q) rm -rf $(addprefix $(BUILD_BASE),$(ALL_SUBMODULES))
	$(Q) $(GIT) checkout $(ALL_SUBMODULES)


.PHONY: docs-clean
docs-clean: ##Clean the Sming documentation
	@echo Cleaning documentation...
	$(Q) $(MAKE) -s -C ../docs clean


##@Building

# Pull in all submodules, regardless of whether they're used or not
.PHONY: submodules
submodules: $(ALL_SUBMODULES:=/.submodule) ##Fetch all third-party submodules (but do not build)

# Build a list of available Components
COMPONENT_SEARCH_DIRS	+= $(ARCH_COMPONENTS) Components Libraries
ALL_COMPONENT_DIRS := $(sort $(foreach d,$(COMPONENT_SEARCH_DIRS),$(call ListSubDirs,$d)))

# Generates a rule to fetch all submodules for a Component
# $1 Component name
# $2 Component path
define GenerateFetchRule
.PHONY: fetch-$1
fetch-$1: $(addsuffix /.submodule,$(filter $2%,$(ALL_SUBMODULES)))
endef

# Add rules for all submodules within components
$(foreach c,$(ALL_COMPONENT_DIRS),$(eval $(call GenerateFetchRule,$(notdir $c),$c)))

FETCH_NAMES := $(filter-out fetch,$(MAKECMDGOALS))

.PHONY: fetch
ifeq (fetch,$(firstword $(MAKECMDGOALS)))
fetch: $(foreach c,$(FETCH_NAMES),fetch-$c)
ifndef FETCH_NAMES
	$(info These are Components with submodules, or which are submodules themselves:)
	$(foreach c,$(ALL_COMPONENT_DIRS),\
		$(if $(filter $c%,$(ALL_SUBMODULES)),\
		$(eval $$(info - $(notdir $c): $c))))
	$(info Fetch one or more components like this: make fetch BMP180 Bounce)
else
	$(foreach c,$(FETCH_NAMES),$(eval $(info $c: found in '$(filter %/$c,$(ALL_COMPONENT_DIRS))')))
	$(error OK)
endif
else
fetch: ##Fetch Component or Library and display location
endif

.PHONY: test
test: Basic_Blink Basic_rBoot Basic_Ssl Basic_HwPWM ##Build a few sample applications

.PHONY: docs
docs: submodules ##Build the Sming documentation
	$(Q) $(MAKE) -C ../docs html


# For integration testing both samples and tests are moved outside of the repo.
SMING_PROJECTS_DIR	?= $(abspath $(SMING_HOME)/..)
SMING_PROJECTS_DIR	:= $(call FixPath, $(SMING_PROJECTS_DIR))
SAMPLES_DIR			:= $(SMING_PROJECTS_DIR)/samples

# Marks sample as build complete for faster re-testing
BUILT_SUFFIX		:= /.$(SMING_SOC).built
SAMPLE_PATHS		= $(sort $(wildcard $(SAMPLES_DIR)/*))
SAMPLES_BUILT		= $(addsuffix $(BUILT_SUFFIX),$(SAMPLE_PATHS))
SAMPLE_NAMES		= $(notdir $(SAMPLE_PATHS))

.PHONY: samples
samples: $(SAMPLES_BUILT) ##Build all sample applications

# Build any samples which haven't yet been marked
$(SAMPLES_BUILT):
	@printf "\n\n** Building $(notdir $(call dirx,$@)) **\n\n"
	$(Q) $(MAKE) --no-print-directory -C $(@D) PIP_ARGS=-q python-requirements sample
	$(Q) touch $@

# Build/clean named samples
# $1 -> Sample path
define CreateSampleTargets
.PHONY: $(notdir $1) $(notdir $1)-clean
$(notdir $1): $1$(BUILT_SUFFIX)
$(notdir $1)-clean:
	-$(Q) cd $1 && $(MAKE) --no-print-directory -C$1 clean
	-$(Q) rm -f $1$(BUILT_SUFFIX)
endef
$(foreach s,$(SAMPLE_PATHS),$(eval $(call CreateSampleTargets,$s)))

# Build component samples
.PHONY: component-samples
component-samples: submodules ##Build all samples contained in components
	$(Q) $(MAKE) --no-print-directory build-component-samples

COMPONENT_SAMPLE_PATHS = $(wildcard $(foreach c,$(COMPONENT_SEARCH_DIRS),$c/*/samples/*))
COMPONENT_SAMPLES_BUILT = $(addsuffix $(BUILT_SUFFIX),$(COMPONENT_SAMPLE_PATHS))

.PHONY: build-component-samples
build-component-samples: $(COMPONENT_SAMPLES_BUILT)

$(COMPONENT_SAMPLES_BUILT):
	@printf "\n\n** Building $(notdir $(call dirx,$@)) **\n\n"
	$(Q) $(MAKE) --no-print-directory -C $(@D) PIP_ARGS=-q python-requirements all
	$(Q) touch $@


PHONY: tests
tests: submodules ##Build and run all tests
	$(Q) $(MAKE) --no-print-directory build-and-run-tests

COMPONENT_TESTS = $(call dirx,$(wildcard $(foreach c,$(COMPONENT_SEARCH_DIRS),$c/*/test/component.mk)))
TEST_PATHS = $(wildcard $(SMING_PROJECTS_DIR)/tests/* $(COMPONENT_TESTS))
TESTS_COMPLETED = $(addsuffix /.$(SMING_SOC).complete,$(TEST_PATHS))

PHONY: build-and-run-tests
build-and-run-tests: $(TESTS_COMPLETED) ##Build and run all test applications

$(TESTS_COMPLETED):
	@printf "\n\n** Building $(notdir $(@D)) **\n\n"
	$(Q) $(MAKE) -C $(@D) execute
	$(Q) touch $@


##@Cleaning

.PHONY: samples-clean
samples-clean: ##Clean all sample applications (all arch/build types)
	@echo Cleaning all samples...
	-$(Q) rm -rf $(addsuffix /out,$(SAMPLE_PATHS)) $(addsuffix /.*.built,$(SAMPLE_PATHS))

.PHONY: component-samples-clean
component-samples-clean: ##Clean all component samples
	@echo Cleaning all component samples...
	-$(Q) rm -rf $(dir $(COMPONENT_SAMPLE_TARGETS))

.PHONY: tests-clean
tests-clean: ##Clean all test applications
	-$(Q) $(foreach p,$(TEST_PATHS),echo ">> $p"; $(MAKE) --no-print-directory -C $p clean;)
	-$(Q) rm -f $(addsuffix /.*.completed,$(TEST_PATHS))


##@Tools

CACHE_VARS += PIP_ARGS
PIP_ARGS ?=
.PHONY: python-requirements
python-requirements: ##Install Python requirements for framework via pip (use PIP_ARGS=... for additional options)
	@echo Installing Python requirements...
	$(Q) $(PYTHON) -m pip install $(PIP_ARGS) -r $(SMING_HOME)/../Tools/requirements.txt


# Recursive wildcard search
# $1 -> list of directories
# $2 -> file extensions filters (using % as wildcard)
define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $2,$d))
endef

# Files that should follow our coding standards

# Root directories to search
CS_ROOT_DIRS	?= $(abspath $(SMING_HOME)/..)
# List of single directories to search
CS_SEARCH_DIRS	?= $(call ListAllSubDirs,$(CS_ROOT_DIRS))
# Resultant set of directories whose contents to apply coding style to
CS_DIRS			?= $(patsubst %/.cs,%,$(wildcard $(foreach d,$(CS_SEARCH_DIRS),$d/.cs)))
# Files to apply coding style to
CS_FILES		= $(if $(CS_DIRS),$(call rwildcard,$(CS_DIRS:=/*),%.cpp %.hpp %.h %.c),)

.PHONY: cs
cs: ##Apply coding style to all core files
	$(call ClangFormat,$(CS_FILES))


CS_DEV_FILES	= $(filter $(abspath $(addprefix ../,$(shell git diff --name-only develop...HEAD))),$(CS_FILES))

.PHONY: cs-dev
cs-dev: ##Apply coding style to all files changed from current upstream develop branch
	$(call ClangFormat,$(CS_DEV_FILES))


##@Help

.PHONY: help
help: ##Show this help summary
	$(PrintHelp)
