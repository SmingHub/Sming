#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
# This makefile is invoked in the application's directory
#
#############################################################

ifndef SMING_HOME
$(error SMING_HOME variable is not set to a valid directory.)
endif

# Don't bother with implicit checks
.SUFFIXES:

.NOTPARALLEL:

.PHONY: all
all: checkdirs submodules ##(default) Build all Component libraries
	$(MAKE) components application

# Load current build type from file
BUILD_TYPE_FILE	:= out/build-type.mk
-include $(BUILD_TYPE_FILE)

#
include $(SMING_HOME)/build.mk

#
# The build system supports several types of configuration variable. The name of the variable needs to be
# added to only one of the xxx_VARS lists below, depending on the required behaviour.
#

# CONFIG_VARS controls when the `App` Component gets rebuilt.
# The values of all these variables are used to create a hash string, which is appended to the library name
# to create a unique variant.
# Components usually add their config variables to COMPONENT_VARS, which also controls variant creation for
# the Component itself. If the variable only affects the application build, not the Component, then it should
# be appended to CONFIG_VARS instead.
# If a Component's external interface isn't affected, but a variant is required, use COMPONENT_RELINK_VARS.
# The values of all CONFIG_VARS are cached after a successful build. A separate cache is maintained for
# each build type (SMING_ARCH, SMING_RELEASE).
CONFIG_VARS		:=

# RELINK_VARS controls when a re-link is required, but no rebuilding is necessary.
# The values of all RELINK_VARS are cached after a successful build.
RELINK_VARS		:=

# CACHE_VARS is a list of variable names which do not affect Component or application building or linking,
# but whose values are cached.
CACHE_VARS		:=

# Use PROJECT_DIR to identify the project source directory, from where this makefile must be included
DEBUG_VARS			+= PROJECT_DIR
PROJECT_DIR			:= $(CURDIR)

ifeq ($(MAKELEVEL),0)
$(info )
$(info $(notdir $(PROJECT_DIR)): Invoking '$(MAKECMDGOALS)' for $(SMING_ARCH) ($(BUILD_TYPE)) architecture)
endif

# CFLAGS used for application and any custom targets
DEBUG_VARS			+= APP_CFLAGS
APP_CFLAGS			=
CPPFLAGS			+= $(APP_CFLAGS)

# Changing USER_CFLAGS will cause an App rebuild automatically, but other Components must be rebuilt manually
CONFIG_VARS			+= USER_CFLAGS

# CFLAGS exported for every Component to use whilst building, including any CUSTOM_TARGETS
DEBUG_VARS			+= GLOBAL_CFLAGS
GLOBAL_CFLAGS = \
	-DSMING_ARCH=$(SMING_ARCH) \
	-DESP_VARIANT=$(ESP_VARIANT) \
	-DPROJECT_DIR=\"$(PROJECT_DIR)\" \
	-DSMING_HOME=\"$(SMING_HOME)\" \
	$(USER_CFLAGS)
CPPFLAGS			+= $(GLOBAL_CFLAGS)

# Targets to be added as dependencies of the application, built directly in this make instance
CUSTOM_TARGETS			:=

# Application libraries will be written here
DEBUG_VARS			+= APP_LIBDIR
APP_LIBDIR			:= $(OUT_BASE)/lib

# Tells linker where to find libraries and scripts
LIBDIRS				:= $(APP_LIBDIR) $(USER_LIBDIR)

# Standard libraries that will be linked with application (Component libraries are defined separately)
LIBS				:= $(EXTRA_LIBS)

# Common linker flags
LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,-Map=$(basename $@).map


# Name of the application to use for link output targets
CACHE_VARS			+= APP_NAME
APP_NAME			?= app

# Firmware memory layout info files
FW_MEMINFO_NEW		:= $(FW_BASE)/fwMeminfo.new
FW_MEMINFO_OLD		:= $(FW_BASE)/fwMeminfo.old
FW_MEMINFO_SAVED	:= out/fwMeminfo

# List of Components we're going to parse, with duplicate libraries removed
COMPONENTS			:= Sming

# For identifying Components which are themselves submodules
ALL_SUBMODULES		= $(ListSubmodules)

#
# Component parsing
#

# Names of Components containing buildable targets, and the full list of targets
BUILDABLE_COMPONENTS :=
ALL_COMPONENT_TARGETS :=

# Prevent rules for custom targets being triggered
COMPONENT_RULE		:= __no_build__

# Component archives (object libraries) to be linked
COMPONENTS_AR		:=

# The set of submodules required by Components
SUBMODULES			:=

# Tracks Components which have been parsed
PARSED_COMPONENTS	:=

# Full list of include directories obtained from all Components (except App)
COMPONENTS_EXTRA_INCDIR	:=

# Components may specify directories containing source code to be compiled with application
APPCODE				:=

# Python requirements.txt collected from components
PYTHON_REQUIREMENTS := 

#
# This macro sets the default component variables before including the (optional) component.mk file.
#
# $1 -> Component name
# $2 -> Component path
# $3 -> Build directory
# $4 -> Output library directory
define ParseComponent
$(if $V,$(info -- Parsing $1))
$(if $2,,$(error Component '$1' not found))
SUBMODULES				+= $(filter $2,$(ALL_SUBMODULES))
CMP_$1_PATH				:= $2
CMP_$1_LIBDIR			:= $4
COMPONENT_LIBDIR		:= $$(CMP_$1_LIBDIR)
COMPONENT_INCDIRS		:= include
COMPONENT_NAME			:= $1
COMPONENT_LIBNAME		:= $1
CMP_$1_BUILD_BASE		:= $3/$1
COMPONENT_BUILD_BASE	:= $$(CMP_$1_BUILD_BASE)
COMPONENT_BUILD_DIR		:= $$(CMP_$1_BUILD_BASE)
COMPONENT_VARS			:=
COMPONENT_RELINK_VARS	:=
COMPONENT_PREREQUISITES	:=
COMPONENT_TARGETS		:=
COMPONENT_DEPENDS		:=
COMPONENT_PYTHON_REQUIREMENTS		:= $$(wildcard $2/requirements.txt)
EXTRA_LIBS				:=
EXTRA_LDFLAGS			:=
# Process any component.mk file (optional)
ifneq (,$(wildcard $2/component.mk))
COMPONENT_PATH			:= $2
COMPONENT_LIBPATH		:= $$(COMPONENT_LIBDIR)/$$(COMPONENT_LIBNAME).a
COMPONENT_SUBMODULES	:=
COMPONENT_APPCODE		:=
include $2/component.mk
CMP_$1_SUBMODULES		:= $$(addprefix $2/,$$(COMPONENT_SUBMODULES))
SUBMODULES				+= $$(CMP_$1_SUBMODULES)
CMP_$1_VARS				:= $$(sort $$(COMPONENT_VARS))
CONFIG_VARS				+= $$(CMP_$1_VARS)
CMP_$1_APPCODE			:= $$(COMPONENT_APPCODE)
LIBS					+= $$(EXTRA_LIBS)
CMP_$1_LDFLAGS			:= $$(EXTRA_LDFLAGS)
LDFLAGS					+= $$(CMP_$1_LDFLAGS)
endif
CMP_$1_PREREQUISITES	:= $$(COMPONENT_PREREQUISITES)
CMP_$1_TARGETS			:= $$(COMPONENT_TARGETS)
CMP_$1_BUILD_DIR		:= $$(COMPONENT_BUILD_DIR)
CMP_$1_LIBNAME			:= $$(COMPONENT_LIBNAME)
CMP_$1_INCDIRS			:= $$(COMPONENT_INCDIRS)
CMP_$1_DEPENDS			:= $$(COMPONENT_DEPENDS)
CMP_$1_RELINK_VARS		:= $$(COMPONENT_RELINK_VARS)
PYTHON_REQUIREMENTS		+= $$(call AbsoluteSourcePath,$2,$$(COMPONENT_PYTHON_REQUIREMENTS))
APPCODE					+= $$(call AbsoluteSourcePath,$2,$$(CMP_$1_APPCODE))
COMPONENTS				+= $$(filter-out $$(COMPONENTS),$$(CMP_$1_DEPENDS))
ifneq (App,$1)
COMPONENTS_EXTRA_INCDIR	+= $$(call AbsoluteSourcePath,$2,$$(CMP_$1_INCDIRS))
# Recursively parse any dependencies
DEPENDENCIES			:= $$(filter-out $$(PARSED_COMPONENTS),$$(CMP_$1_DEPENDS))
ifneq (,$$(DEPENDENCIES))
PARSED_COMPONENTS		+= $$(DEPENDENCIES)
$$(call ParseComponentList,$$(DEPENDENCIES))
endif
endif # App
endef # ParseComponent

# Build a list of all available Components
ALL_COMPONENT_DIRS = $(foreach d,$(ALL_SEARCH_DIRS),$(call ListSubDirs,$d))

# Lookup Component directory from a name
# $1 -> Component name
define FindComponentDir
$(if $(subst Sming,,$1),$(firstword $(filter %/$1,$(ALL_COMPONENT_DIRS))),$(SMING_HOME))
endef

# Parse a list of Components
# $1 -> List of Component names
define ParseComponentList
$(foreach c,$1,$(eval $(call ParseComponent,$c,$(call FindComponentDir,$c),$(SMING_HOME)/$(BUILD_BASE),$(USER_LIBDIR))))
endef

# Must parse the Application Component first to get project dependencies
$(eval $(call ParseComponent,App,$(CURDIR),$(BUILD_BASE),$(abspath $(APP_LIBDIR))))

# Load cached configuration variables. On first run this file won't exist, so all values
# will be as specified by defaults or in project's component.mk file.
# Values may be overriden via command line to update the cache.
# If file has become corrupted it will prevent cleaning, so make this conditional.
CONFIG_CACHE_FILE	:= $(OUT_BASE)/config.mk
ifndef MAKE_CLEAN
-include $(CONFIG_CACHE_FILE)
endif

# Append standard search directories to any defined by the application
ALL_SEARCH_DIRS			:= $(call FixPath,$(abspath $(COMPONENT_SEARCH_DIRS)))
COMPONENTS_EXTRA_INCDIR	+= $(ALL_SEARCH_DIRS)
ALL_SEARCH_DIRS			+= $(ARCH_COMPONENTS) $(SMING_HOME)/Components $(SMING_HOME)/Libraries

# And add in any requested libraries
COMPONENTS				+= $(sort $(ARDUINO_LIBRARIES))

# Pull in all Component definitions
$(eval $(call ParseComponentList,$(COMPONENTS)))

# Resolve dependencies to a depth of 1
# $1 -> Component name
define ResolveDependentComponents
$(CMP_$1_DEPENDS) $(foreach c,$(CMP_$1_DEPENDS),$(CMP_$c_DEPENDS))
endef

# Resolve component and variable dependences but limit recursion as components may be mutually dependent
# $1 -> Component name
define ResolveDependencies
CMP_$1_DEPENDS	:= $(call ResolveDependentComponents,$1)
CMP_$1_DEPENDS	:= $(call ResolveDependentComponents,$1)
CMP_$1_DEPENDS	:= $(call ResolveDependentComponents,$1)
CMP_$1_DEPENDS	:= $(call ResolveDependentComponents,$1)
CMP_$1_DEPENDS	:= $$(sort $$(filter-out $1,$$(CMP_$1_DEPENDS)))
CMP_$1_ALL_VARS	:= $$(sort $(CMP_$1_VARS) $$(foreach c,$$(CMP_$1_DEPENDS),$$(CMP_$$c_VARS)) $(CMP_$1_RELINK_VARS))
endef

$(foreach c,$(COMPONENTS),$(eval $(call ResolveDependencies,$c)))


# Check number of matches for a Component: should be exactly 1
# $1 => Component name
define CheckComponentMatches
ifneq ($1,Sming)
COMPONENT_MATCHES := $(sort $(filter %/$1,$(ALL_COMPONENT_DIRS)))
ifeq ($$(words $$(COMPONENT_MATCHES)),0)
$$(warning No matches found for Component '$1')
else ifneq ($$(words $$(COMPONENT_MATCHES)),1)
$$(warning Multiple matches found for Component '$1':)
$$(foreach m,$$(COMPONENT_MATCHES),$$(info - $$m))
endif
endif
endef

$(foreach c,$(COMPONENTS),$(eval $(call CheckComponentMatches,$c)))


# This macro assigns a library and build path based on a hash of the component variables
# $1 -> Component name
define ParseComponentLibs
ifneq (,$$(CMP_$1_LIBNAME))
ifeq (,$$(CMP_$1_ALL_VARS))
CMP_$1_LIBHASH			:=
COMPONENT_VARIANT		:= $$(CMP_$1_LIBNAME)
else
COMPONENT_VARIABLES		:= $$(foreach v,$$(CMP_$1_ALL_VARS),$$($$v)=$$($$($$v)))
CMP_$1_LIBHASH			:= $$(call CalculateVariantHash,COMPONENT_VARIABLES)
COMPONENT_VARIANT		:= $$(CMP_$1_LIBNAME)-$$(CMP_$1_LIBHASH)
endif
ifneq ($$(COMPONENT_VARIANT),$1)
CMP_$1_BUILD_DIR		:= $$(CMP_$1_BUILD_DIR)/$$(COMPONENT_VARIANT)
endif
COMPONENT_LIBPATH		:= $$(CMP_$1_LIBDIR)/$(CLIB_PREFIX)$$(COMPONENT_VARIANT).a
CMP_$1_TARGETS			+= $$(COMPONENT_LIBPATH)
COMPONENTS_AR			+= $$(COMPONENT_LIBPATH)
endif
ifneq (,$$(CMP_$1_TARGETS))
BUILDABLE_COMPONENTS	+= $1
ALL_COMPONENT_TARGETS	+= $$(CMP_$1_TARGETS)
endif
endef

# Order unimportant so sort for ease of reading and remove duplicates
CONFIG_VARS				:= $(sort $(CONFIG_VARS))
RELINK_VARS				+= $(foreach c,$(COMPONENTS),$(CMP_$c_RELINK_VARS))

# Always build App last, using a variant based on all config variables
# Note that a link step is always performed, so nothing needs to be done with RELINK_VARS
COMPONENTS				+= App
CMP_App_VARS			:= $(CONFIG_VARS)
CMP_App_ALL_VARS		:= $(CONFIG_VARS)
$(foreach c,$(COMPONENTS),$(eval $(call ParseComponentLibs,$c)))

export COMPONENTS_EXTRA_INCDIR
export APPCODE
export APP_CFLAGS
export GLOBAL_CFLAGS
export CONFIG_VARS

# Export all config variables
EXPORT_VARS := $(sort $(CONFIG_VARS) $(CACHE_VARS) $(RELINK_VARS))
$(foreach v,$(EXPORT_VARS),$(eval export $v))


##@Building

COMPONENT_DIRS := $(foreach d,$(ALL_SEARCH_DIRS),$(wildcard $d/*))

%/component.mk:
	@if [ -f $(@D)/../.patches/$(notdir $(@D))/component.mk ]; then \
		echo Patching $(abspath $(@D)/../.patches/$(notdir $(@D))/component.mk); \
		cp $(@D)/../.patches/$(notdir $(@D))/component.mk $@; \
	fi

SUBMODULES_FOUND = $(wildcard $(SUBMODULES:=/.submodule))
SUBMODULES_FETCHED = $(filter-out $(SUBMODULES_FOUND:/.submodule=),$(SUBMODULES))

.PHONY: submodules
submodules: | $(COMPONENT_DIRS:=/component.mk) $(SUBMODULES:=/.submodule) ##Recursively fetch all required submodules
ifneq (,$V)
	$(call PrintVariable,SUBMODULES_FETCHED)
endif
ifneq ($(SUBMODULES_FETCHED),)
	$(Q) $(MAKE) -s --no-print-directory submodules
endif


# Define target for building a component library
# We add a pseudo-target for each Component (using its name) to (re)build all contained targets
# e.g. spiffs: libspiffs.a spiffy.exe
# $1 -> Component name
define GenerateComponentTargets

.PHONY: $1-build
$1-build: $(addsuffix -build,$(filter $(CMP_$1_DEPENDS),$(BUILDABLE_COMPONENTS))) | $(CMP_$1_BUILD_DIR) $(CMP_$1_SUBMODULES:=/.submodule)
	@echo
	@echo Building $(CMP_$1_TARGETS)
	+$(Q) $(MAKE) -r -R --no-print-directory -C $(CMP_$1_BUILD_DIR) -f $(SMING_HOME)/component-wrapper.mk \
		COMPONENT_NAME=$1 \
		COMPONENT_PATH=$(CMP_$1_PATH) \
		COMPONENT_BUILD_BASE=$(CMP_$1_BUILD_BASE) \
		COMPONENT_LIBDIR=$(CMP_$1_LIBDIR) \
		COMPONENT_LIBNAME=$(CMP_$1_LIBNAME) \
		COMPONENT_LIBHASH=$(CMP_$1_LIBHASH) \
		$(SUBMAKE_FLAGS) \
		build

$(CMP_$1_BUILD_DIR):
	$(Q) mkdir -p $$@

.PHONY: $1-rebuild
$1-rebuild: $1-clean $1-build

.PHONY: $1-remove-targets
$1-remove-targets:
	-$(Q) rm -f $(CMP_$1_TARGETS)

.PHONY: $1-clean
$1-clean: $1-remove-targets
	-$(Q) rm -rf $(CMP_$1_BUILD_BASE)

$(foreach t,$(CMP_$1_TARGETS),$(eval $(call GenerateComponentTargetRule,$1,$t)))

endef # GenerateComponentTargets

# By default, the sub-make for all Components (except App) is invoked only if any of the declared targets don't exist
# This means if a file changes in the Component, it won't get rebuilt automatically.
# When working on a Component, add its name to FULL_COMPONENT_BUILD so it always gets checked.
# To add all Components, set `FULL_COMPONENT_BUILD=$(COMPONENTS)`
CACHE_VARS += FULL_COMPONENT_BUILD
FULL_COMPONENT_BUILD ?=
ifeq ($(FULL_COMPONENT_BUILD),1)
	FULL_COMPONENT_BUILD = $(COMPONENTS)
endif

# Each Component's real target is built using the phony -build target above, which we define here as a pre-requisite
# but only for the `App` Component and for any targets which don't actually exist.
# Components may always be explicitly (re)built using the -build or -rebuild targets.
# Note that if we don't touch the target then make won't know it's been updated and won't re-link the firmware
# until its next run. The side-effect of this is that a link will always happen, even if it's not required.
# $1 -> Component name
# $2 -> target
define GenerateComponentTargetRule
ifeq (App,$1)
$2: $1-build
	$(Q) touch $$@
else ifeq (,$(wildcard $2))
$2: $1-build
	$(Q) touch $$@
else ifneq (,$(filter $1,$(FULL_COMPONENT_BUILD)))
$2: $1-build
	$(Q) touch $$@
endif
endef


# Create rules for all Components
$(foreach c,$(BUILDABLE_COMPONENTS),$(eval $(call GenerateComponentTargets,$c)))

# The arch-specific targets, including link and firmware image creation
include $(ARCH_BASE)/app.mk

.PHONY: rebuild
rebuild: clean all ##Re-build application

.PHONY: checkdirs
checkdirs: | $(BUILD_BASE) $(FW_BASE) $(TOOLS_BASE) $(APP_LIBDIR) $(USER_LIBDIR)

$(BUILD_BASE) $(FW_BASE) $(TOOLS_BASE) $(APP_LIBDIR) $(USER_LIBDIR):
	$(Q) mkdir -p $@

# Targets to be built before anything else (e.g. source code generators)
PREREQUISITES := $(foreach c,$(COMPONENTS),$(CMP_$c_PREREQUISITES))

# Build all Component (user) libraries
.PHONY: components
components: $(PREREQUISITES) $(ALL_COMPONENT_TARGETS) $(CUSTOM_TARGETS)

##@Cleaning

SMING_MAKE := $(Q) $(MAKE) --no-print-directory -C $(SMING_HOME)

.PHONY: dist-clean
dist-clean: ##Clean everything (all arch/build types)
	$(SMING_MAKE) $@

.PHONY: submodules-clean
submodules-clean: ##Reset state of all third-party submodules
	$(SMING_MAKE) $@

.PHONY: components-clean
components-clean: ##Remove generated Component libraries
	$(SMING_MAKE) $@

.PHONY: config-clean
config-clean: ##Clear build configuration, so next make will use original defaults
	@echo Cleaning build configuration
	-$(Q) rm -f $(CONFIG_CACHE_FILE)

.PHONY: clean
clean: ##Remove all generated build files (but leave build config intact)
	@echo Cleaning application...
	-$(Q) rm -rf $(BUILD_BASE) $(FW_BASE) $(APP_LIBDIR)

##@Tools

.PHONY: cs
cs: .clang-format ##Apply coding style to selected project directories
	$(SMING_MAKE) cs CS_ROOT_DIRS=$(PROJECT_DIR)

.clang-format:
	$(Q) cp $(SMING_HOME)/../.clang-format $@

.PHONY: cs-dev
cs-dev: ##Apply coding style to all files changed from current upstream develop branch
	$(SMING_MAKE) $@

.PHONY: gdb
gdb: kill_term ##Run the debugger console
	$(GDB_CMDLINE)

.PHONY: fetch
fetch: ##Fetch Component or Library and display location
	$(SMING_MAKE) $(MAKECMDGOALS)

# Stack trace decoder
CACHE_VARS += TRACE
TRACE ?=
.PHONY: decode-stacktrace
decode-stacktrace: ##Open the stack trace decoder ready to paste dump text. Alteratively, use `make decode-stacktrace TRACE=/path/to/crash.stack`
	$(Q) if [ -z "$(TRACE)" ]; then \
		echo "Decode stack trace: Paste stack trace here"; \
	fi
	$(Q) $(PYTHON) $(ARCH_TOOLS)/decode-stacktrace.py $(TARGET_OUT_0) $(TRACE)


CACHE_VARS += PIP_ARGS
PIP_ARGS ?=
.PHONY: python-requirements
python-requirements: ##Install Python requirements of project via pip (use PIP_ARGS=... for additional options)
ifeq (,$(PYTHON_REQUIREMENTS))
	@echo No Python requirements to install for this project.
else
	@echo Installing Python requirements...
	$(Q) $(PYTHON) -m pip install $(PIP_ARGS) $(foreach reqfile,$(PYTHON_REQUIREMENTS),-r $(reqfile))
endif

##@Testing

# OTA Server
CACHE_VARS			+= SERVER_OTA_PORT
SERVER_OTA_PORT		?= 9999
.PHONY: otaserver
otaserver: all ##Launch a simple python HTTP server for testing OTA updates
	$(info Starting OTA server for TESTING)
	$(Q) cd $(FW_BASE) && $(PYTHON) -m SimpleHTTPServer $(SERVER_OTA_PORT)

##@Help

.PHONY: list-config
list-config: ##Print the contents of build variables
	$(info )
	$(info ** Sming build configuration **)
	$(info )
	$(if $(V),$(call PrintVariable,MAKEFILE_LIST))
	$(call PrintVariableSorted,PREREQUISITES)
	$(call PrintVariableSorted,CUSTOM_TARGETS)
	$(call PrintVariableSorted,LIBS)
	$(call PrintVariableSorted,ARDUINO_LIBRARIES)
	$(call PrintVariableSorted,SUBMODULES)
	$(if $(V),$(call PrintVariableSorted,ALL_SUBMODULES))
	$(call PrintVariableRefs,CONFIG_VARS)
	$(call PrintVariableRefs,CACHE_VARS)
	$(call PrintVariableRefs,RELINK_VARS)
	$(call PrintVariableRefs,DEBUG_VARS)
	$(info )
	$(info )

nullstr :=

# Print component information
# $1 -> Component name
define PrintComponentInfo
	$(info > $1: $(CMP_$1_PATH))
	$(if $(V),
		$(if $(CMP_$1_SUBMODULES),$(info $(nullstr)    Submodules: $(notdir $(CMP_$1_SUBMODULES))))
		$(if $(CMP_$1_DEPENDS),$(info $(nullstr)    Depends: $(CMP_$1_DEPENDS)))
		$(if $(CMP_$1_APPCODE),$(info $(nullstr)    Appcode: $(CMP_$1_APPCODE)))
		$(if $(CMP_$1_TARGETS),$(info $(nullstr)    Targets: $(notdir $(CMP_$1_TARGETS))))
		$(if $(CMP_$1_ALL_VARS),\
			$(info $(nullstr)    Variables:)\
			$(foreach v,$(CMP_$1_ALL_VARS),\
				$(info $(nullstr)    $(if $(filter $v,$(CMP_$1_VARS) $(CMP_$1_RELINK_VARS)), ,i) $v=$($v)) ))
			)
endef

.PHONY: list-components
list-components: ##Print details of all Components for this project
	$(call PrintVariable,ALL_SEARCH_DIRS)
	$(if $(V),$(call PrintVariable,ALL_COMPONENT_DIRS))
	$(info Components:)
	$(foreach c,$(sort $(COMPONENTS)),$(eval $(call PrintComponentInfo,$c)))

# Dump content of requirements.txt file
# $1 -> absolute path to file
define DumpRequirementsTxt
	@echo \# From $1:
	@cat $1

endef
.PHONY: list-python-requirements
list-python-requirements: ##List Python requirements for this project
ifeq (,$(PYTHON_REQUIREMENTS))
	@echo \# No Python requirements for this project.
else
	$(foreach reqfile,$(PYTHON_REQUIREMENTS),$(call DumpRequirementsTxt,$(reqfile)))
endif

# => Help
.PHONY: help
help: ##Show this help summary
	$(PrintHelp)
ifneq (,$V)
	@echo 'Each Component also has the following targets:'
	@echo '  cmp-build'
	@echo '  cmp-clean'
	@echo '  cmp-rebuild'
endif


# Called at final output stage to write a copy of config variables used for the build
# $1 -> Name of output file
define WriteFirmwareConfigFile
$(call WriteConfig,$1.cfg,$(CONFIG_VARS) $(RELINK_VARS) $(CACHE_VARS))
endef

# Write config variables to a cache file
# $1 -> Output filename
# $2 -> List of variable names to output
define WriteCacheValues
$(shell	mkdir -p $(dir $1);
	echo '# Automatically generated file. Do not edit.' > $1;
	echo >> $1;
	$(foreach v,$2,echo '$v=$(value $v)' >> $1;) )
endef

# Update build type cache
$(eval $(call WriteCacheValues,$(BUILD_TYPE_FILE),SMING_ARCH ESP_VARIANT SMING_RELEASE STRICT))

# Update config cache file
# We store the list of variable names to ensure that any not actively in use don't get lost
# $1 -> Output filename
# $2 -> Name of variable which contains list of names
define WriteConfigCache
$(call WriteCacheValues,$1,$($2))
$(shell	echo >> $1;
		echo '$2 := $($2)' >> $1 )
endef

# Update variable cache for all operations except cleaning
ifndef MAKE_CLEAN
CACHED_VAR_NAMES := $(sort $(CACHED_VAR_NAMES) $(CONFIG_VARS) $(RELINK_VARS) $(CACHE_VARS))
$(eval $(call WriteConfigCache,$(CONFIG_CACHE_FILE),CACHED_VAR_NAMES))
endif
