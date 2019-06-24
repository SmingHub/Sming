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
dist-clean: submodules-clean samples-clean ##Clean everything for all arch/build types
	-$(Q) rm -rf out

.PHONY: clean
clean: ##Remove all intermediate build files, but leave generated libraries and tools
	-$(Q) rm -rf $(BUILD_BASE)

.PHONY: components-clean
components-clean: ##Remove generated Component (user) libraries
	@echo Cleaning all Component libraries
	-$(Q) rm -f $(USER_LIBDIR)/*
	-$(Q) rm -rf $(OUT_BASE)

.PHONY: submodules-clean
submodules-clean: ##Reset state of all third-party submodules
	@echo Cleaning all submodules...
	-$(Q) rm -rf $(ListSubmodules)
	-$(Q) rm -rf $(addprefix $(BUILD_BASE),$(ListSubmodules))
	$(Q) $(GIT) checkout $(ListSubmodules)


##@Building

.PHONY: test
test: Basic_Blink Basic_rBoot Basic_Ssl Basic_HwPWM ##Build a few sample applications


# => Documentation
DOCS_DIR := ../docs

.PHONY: wiki api docs
docs: wiki api ##Build the Sming API and Wiki documentation
wiki: $(DOCS_DIR)/wiki/Home.md
api: $(DOCS_DIR)/api/sming/index.html

$(DOCS_DIR)/wiki/Home.md:
	$(vecho) "No Wiki submodule found. Using git to fetch it..."
	$(Q) $(GIT) submodule update --init $(DOCS_DIR)/wiki

$(DOCS_DIR)/api/error.log:
ifndef DOXYGEN
	$(error doxygen not found - not building API docs)
endif
	$(Q) mkdir -p $(DOCS_DIR)/api
	$(Q) cd $(DOCS_DIR) && doxygen 2>api/stderr.log 1>api/doxygen.log

$(DOCS_DIR)/api/sming/index.html: $(DOCS_DIR)/api/error.log
	$(vecho) "Undocumented compounds: $(shell grep "Compound.*is not documented" $(DOCS_DIR)/api/error.log | wc -l)"
	$(vecho) "Undocumented members:  $(shell grep "Member.*is not documented" $(DOCS_DIR)/api/error.log | wc -l) "
	$(vecho) "Undocumented parameters: $(shell grep "The following parameters of .* are not documented" $(DOCS_DIR)/api/error.log | wc -l) "
	$(vecho) "Open in your browser: file://$(abspath $@)"


##@Building

SAMPLES_DIRS = $(shell ls -1 ../samples)

.PHONY: samples $(SAMPLES_DIRS)
samples: | $(SAMPLES_DIRS) ##Build all sample applications

$(SAMPLES_DIRS):
	$(Q) $(MAKE) --no-print-directory -C ../samples/$@


##@Cleaning

.PHONY: samples-clean
samples-clean: ##Clean all sample applications (all arch/build types)
	@echo Cleaning all samples...
	-$(Q) cd ../samples && rm -rf $(addsuffix /out,$(SAMPLES_DIRS))


##@Developer

# Recursive wildcard search
# $1 -> list of directories
# $2 -> file extensions filters (using % as wildcard)
define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $2,$d))
endef

# Files that should follow our coding standards
CS_DIRS		= Core Platform Arch/*/Core Arch/*/Platform ../samples
CS_FILES	= $(call rwildcard,$(patsubst %,%/*,$(CS_DIRS)),%.cpp %.h %.c)

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
