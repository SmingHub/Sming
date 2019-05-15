#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
# Multi-architecture support by mikee47 <mike@sillyhouse.net>
#
#############################################################

ifndef SMING_HOME
$(error SMING_HOME variable is not set to a valid directory.)
endif

## Defaults for Sming Core

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming

# MacOS / Linux
# SMING_HOME = /opt/esp-open-sdk

##@Building

.PHONY: all
all: checkdirs libsming tools ##(default) Build the Sming library, user libraries, Arduino libraries and tools

include $(SMING_HOME)/build.mk

# name for the target project
TARGET := lib$(LIBSMING)

RELOAD_MKFILE = 0

# List of directories containing source code to build
MODULES :=

# List of dependent submodule directories which may require updating/patching
SUBMODULES :=

#
SAMPLES_DIRS := $(shell ls -1 ../samples)
DOXYGEN	:= $(shell command -v doxygen 2> /dev/null)

# => WPS
CONFIG_VARS += ENABLE_WPS
ifeq ($(ENABLE_WPS), 1)
   CFLAGS += -DENABLE_WPS=1
endif

# => http-parser
SUBMODULES		+= $(COMPONENTS)/http-parser
MODULES			+= $(COMPONENTS)/http-parser

# => libb64
MODULES			+= $(COMPONENTS)/libb64

# => websocket-parser
SUBMODULES		+= $(COMPONENTS)/ws_parser
MODULES			+= $(COMPONENTS)/ws_parser

# => mqtt-codec
SUBMODULES		+= $(COMPONENTS)/mqtt-codec
MODULES			+= $(COMPONENTS)/mqtt-codec/src
EXTRA_INCDIR	+= $(COMPONENTS)/mqtt-codec/src


# => yuarel
SUBMODULES		+= $(COMPONENTS)/libyuarel
MODULES			+= $(COMPONENTS)/libyuarel

include $(ARCH_BASE)/sming.mk

# Get list of subdirectory paths
# $1 -> Root directory
define ScanModuleDirs
	$(sort $(dir $(wildcard $1/*/ $1/*/*/ $1/*/*/*/)))
endef

# If specified, restrict importing and building to a specific list of libraries
ARDUINO_LIBRARIES	?= $(patsubst Libraries/%,%,$(wildcard Libraries/*/))
ARDUINO_LIBRARIES	:= $(ARDUINO_LIBRARIES:/=)

# Optionally, apply library exclusion list (e.g. not compatible with architecture)
EXCLUDE_LIBRARIES := $(filter $(EXCLUDE_LIBRARIES),$(ARDUINO_LIBRARIES))
ifneq (,$(EXCLUDE_LIBRARIES))
ARDUINO_LIBRARIES := $(filter-out $(EXCLUDE_LIBRARIES),$(ARDUINO_LIBRARIES))
ifeq ("$(V)","1")
$(warning Excluded libraries: $(EXCLUDE_LIBRARIES))
endif
endif

# Identify which libraries have submodules
LIBRARY_SUBMODULES	:= $(call ListSubmodules,Libraries)
LIBRARY_SUBMODULES	:= $(patsubst Libraries/%,%,$(LIBRARY_SUBMODULES))
LIBRARY_SUBMODULES	:= $(filter $(LIBRARY_SUBMODULES), $(ARDUINO_LIBRARIES))
# List of any Arduino library .submodule targets
LIBRARY_SUBMODULES	:= $(foreach lib,$(LIBRARY_SUBMODULES),Libraries/$(lib)/.submodule)

# which modules (subdirectories) to include in compiling
SMING_MODULES	:= $(ARCH_CORE) $(ARCH_BASE)/Platform Platform Core
SMING_MODULES	:= $(foreach mod,$(SMING_MODULES),$(call ScanModuleDirs,$(mod)))
MODULES			:= $(ARCH_SYS) System Wiring $(SMING_MODULES) $(MODULES)
MODULES			+= $(wildcard Services/*/)
MODULES			+= $(foreach lib,$(ARDUINO_LIBRARIES),Libraries/$(lib) Libraries/$(lib)/src)
MODULES			:= $(MODULES:/=)

EXTRA_INCDIR	:=	$(ARCH_BASE) $(ARCH_CORE) $(ARCH_SYS)/include \
					$(ARCH_COMPONENTS) $(COMPONENTS) System/include Wiring Core \
					. $(EXTRA_INCDIR)

include $(SMING_HOME)/modules.mk


CUSTOM_TARGETS += $(LIBS)

# Define secondary pseudo-targets for all libraries
define UserLibrary
.PHONY $1:
$1: $(call UserLibPath,$1)
endef
$(foreach lib,$(LIBS),$(eval $(call UserLibrary,$(lib))))


.PHONY: libsming
libsming: $(LIBSMING_DST)

$(LIBSMING_DST): $(APP_AR) $(CUSTOM_TARGETS) ##Build the Sming framework and user libraries
	$(vecho) "Installing '$(LIBSMING)' library"
ifeq ($(ENABLE_SSL), 1)
	$(vecho) "+ SSL support is enabled"
else
	$(vecho) "(!) SSL support is not enabled. To enable it type: 'make clean; make ENABLE_SSL=1'"
endif
	$(Q) cp -r $(APP_AR) $(LIBSMING_DST)
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cr $@ $^

.PHONY: checkdirs
checkdirs: submodules reload | $(BUILD_DIR) $(USER_LIBDIR)

$(BUILD_DIR) $(USER_LIBDIR):
	$(Q) mkdir -p $@

##@Cleaning

.PHONY: dist-clean
dist-clean: clean submodules-clean tools-clean user-lib-clean samples-clean ##Clean everything

.PHONY: clean
clean: $(CLEAN) ##Remove all generated build files
	$(Q) rm -f $(APP_AR)
	$(Q) rm -rf $(BUILD_BASE)

.PHONY: user-lib-clean
user-lib-clean: ##Remove generated user libraries
	-$(Q) rm -f $(wildcard $(USER_LIBDIR)/lib*.a) $(APP_AR)
	-$(Q) $(GIT) checkout $(USER_LIBDIR)

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
	$(Q) cd $(DOCS_DIR); doxygen 2>api/stderr.log 1>api/doxygen.log

$(DOCS_DIR)/api/sming/index.html: $(DOCS_DIR)/api/error.log
	$(vecho) "Undocumented compounds: $(shell grep "Compound.*is not documented" $(DOCS_DIR)/api/error.log | wc -l)"
	$(vecho) "Undocumented members:  $(shell grep "Member.*is not documented" $(DOCS_DIR)/api/error.log | wc -l) "
	$(vecho) "Undocumented parameters: $(shell grep "The following parameters of .* are not documented" $(DOCS_DIR)/api/error.log | wc -l) "
	$(vecho) "Open in your browser: file://$(abspath $@)"


# Apply patch to a submodule
# $1 -> patch file with relative path
define ApplyPatch
	touch $1; \
	$(GIT) apply -v $1 --ignore-whitespace --whitespace=nowarn
endef

# If there's a patch for this submodule, apply it
# We look for patch in .. and in ../.patches
# $1 -> submodule path
# $2 -> name of patch file
define TryApplyPatch
	cd $1; \
	if [ -f ../$2 ]; then \
		$(call ApplyPatch,../$2); \
	elif [ -f ../.patches/$2 ]; then \
		$(call ApplyPatch,../.patches/$2); \
	fi
endef

.PHONY: submodules submodules-clean
submodules: | $(LIBRARY_SUBMODULES) $(addsuffix /.submodule,$(SUBMODULES)) ##Fetch all third-party submodules (but do not build)

##@Cleaning
submodules-clean: ##Reset state of all third-party submodules
	-$(Q) rm -rf $(call ListSubmodules)
	-$(Q) rm -rf $(addprefix $(BUILD_BASE),$(call ListSubmodules))
	$(GIT) checkout Components
	$(GIT) checkout Libraries

# Update and patch submodule
# Patch file is either in submodule parent directory itself or subdirectory .patches from there
%/.submodule:
	$(vecho) "Fetching $(dir $@) ..."
	$(Q) mkdir -p $(dir $@)
	$(Q) $(GIT) submodule update --init --force --recursive $(dir $@)
	$(Q) $(call TryApplyPatch,$(dir $@),$(notdir $(patsubst %/,%,$(dir $@))).patch)
	$(Q) touch $@
    RELOAD_MKFILE=1

    
# if the new submodule brings source code files that need to be compiled inside Sming
# then we need somehow to be able to "see" these new files.
# For now we solve this by "reloading" the makefile after fetching a module.
.PHONY: reload
reload:
	$(Q) if [ $(RELOAD_MKFILE) -eq 1 ]; then \
		echo "Reloading..."; \
        $(MAKE) -C $(SMING_HOME) $(MAKECMDGOALS) RELOAD_MKFILE=0; \
    fi

##@Building
.PHONY: samples $(SAMPLES_DIRS) samples-clean
samples: | $(SAMPLES_DIRS) ##Build all sample applications

$(SAMPLES_DIRS):
	$(Q) $(MAKE) -C $(SMING_HOME)/../samples/$@

##@Cleaning
samples-clean: ##Invoke 'clean' on all sample applications
	-$(Q)	for dir in $(SAMPLES_DIRS); do \
				$(MAKE) -C $(SMING_HOME)/../samples/$$dir clean; \
			done

##@Building
.PHONY: tools
tools: $(TOOLS) ##Build all required internal tools

##@Cleaning
.PHONY: tools-clean
tools-clean: $(TOOLS_CLEAN) ##@Cleaning Remove intermediate files created when building tools

##@Building
.PHONY: rebuild
rebuild: clean all ##Re-build the Sming libraries

# Recursive wildcard search
# $1 -> directory
# $2 -> file extensions
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# Files that should follow our coding standards
CS_DIRS		:= Core Platform Arch/*/Core Arch/*/Platform ../samples
CS_FILES	:= $(call rwildcard,$(patsubst %,%/*,$(CS_DIRS)),*.cpp *.h *.c)

##@Developer

.PHONY: cs
cs: ##Apply coding style to all core files
	$(Q) for FILE in $(CS_FILES); do \
		clang-format -i -style=file $$FILE; \
	done

##@Help

.PHONY: list-config
list-config: ##Print the contents of internal build variables
	$(call ListConfig)

.PHONY: help
help: ##Show this help summary
	$(call PrintHelp)
