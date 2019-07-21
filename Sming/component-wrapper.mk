#######################
#
# This file is invoked as a separate MAKE instance to build each Component.
# (The idea was borrowed from Espressif's IDF, but is much simpler.)
#
# By default, a Component builds a single library.
# The working directory (CURDIR) is set to the Component build directory.
# All required submodules are pulled in and patched before this makefile is invoked.
# See `building.md` for further details.
#
#######################

.PHONY: all
all:
	$(error Internal makefile) 

include $(SMING_HOME)/build.mk

# Makefile runs in the build directory
COMPONENT_BUILD_DIR := $(CURDIR)

CFLAGS				:= $(CFLAGS) $(GLOBAL_CFLAGS)

#
CUSTOM_BUILD		:=
COMPONENT_TARGETS	:=
EXTRA_OBJ			:=
COMPONENT_CFLAGS	:=
COMPONENT_CXXFLAGS	:=
COMPONENT_VARS		:=
COMPONENT_LIBNAME	:= $(COMPONENT_NAME)

ifeq (,$(wildcard $(COMPONENT_PATH)/Makefile-user.mk))
# Regular Component
ifeq (App,$(COMPONENT_NAME))
COMPONENT_SRCDIRS	:= app
COMPONENT_INCDIRS	:= include
else
COMPONENT_SRCDIRS	:= . src
endif
COMPONENT_SRCFILES	:=
else
# Legacy project
MODULES				:= app
EXTRA_INCDIR		:= include
EXTRA_SRCFILES		:=
include $(COMPONENT_PATH)/Makefile-user.mk
COMPONENT_SRCDIRS	:= $(MODULES)
COMPONENT_SRCFILES	:= $(EXTRA_SRCFILES)
endif

ifeq (App,$(COMPONENT_NAME))
CFLAGS				+= $(APP_CFLAGS)
else ifneq ($(STRICT),1)
# Enforce strictest building for regular Components and treat as errors
CFLAGS				:= $(filter-out -Wno-sign-compare -Wno-strict-aliasing,$(CFLAGS)) -Werror
CXXFLAGS			:= $(filter-out -Wno-reorder,$(CXXFLAGS))
endif

INCDIR				= $(EXTRA_INCDIR) $(COMPONENTS_EXTRA_INCDIR)

# Build a Component target using MAKE
# The makefile should accept TARGET and BUILD_DIR variables
# $1 -> path to makefile, relative to Component path
# $2 -> parameters
define MakeTarget
$(Q) mkdir -p $(COMPONENT_BUILD_DIR)/$(basename $(@F)) $(@D)
+$(Q) $(MAKE) --no-print-directory -C $(dir $(COMPONENT_PATH)/$1) -f $(notdir $1) \
	TARGET=$@ BUILD_DIR=$(COMPONENT_BUILD_DIR)/$(basename $(@F)) V=$(V) $2
endef

# Define variables required for custom builds
COMPONENT_VARIANT := $(COMPONENT_LIBNAME)$(if $(COMPONENT_LIBHASH),-$(COMPONENT_LIBHASH))
COMPONENT_LIBPATH := $(COMPONENT_LIBDIR)/$(CLIB_PREFIX)$(COMPONENT_VARIANT).a

# component.mk is optional
-include $(COMPONENT_PATH)/component.mk

ifeq (App,$(COMPONENT_NAME))
COMPONENT_SRCDIRS	+= $(APPCODE)
EXTRA_INCDIR		:= $(call AbsoluteSourcePath,$(COMPONENT_PATH),$(EXTRA_INCDIR) $(COMPONENT_INCDIRS))
endif

# COMPONENT_LIBNAME gets undefined if Component doesn't create a library
ifneq (,$(COMPONENT_LIBNAME))

COMPONENT_TARGETS += $(COMPONENT_LIBPATH)

# List of directories containing object files
BUILD_DIRS :=

ifeq (App,$(COMPONENT_NAME))
$(eval $(call WriteConfig,$(COMPONENT_BUILD_DIR)/build.cfg,$(CONFIG_VARS)))
else ifneq (,$(COMPONENT_VARS))
$(eval $(call WriteConfig,$(COMPONENT_BUILD_DIR)/build.cfg,$(COMPONENT_VARS)))
endif

# Custom build means we don't need any of the regular build logic, that's all be done in component.mk
ifeq (,$(CUSTOM_BUILD))

CFLAGS		+= $(COMPONENT_CFLAGS)
CXXFLAGS	+= $(COMPONENT_CXXFLAGS)

# $1 -> absolute source directory, no trailing path separator
# $2 -> relative output build directory, with trailing path separator
define GenerateCompileTargets
BUILD_DIRS += $2
ifneq (,$(filter $1/%.s,$(SOURCE_FILES)))
$2%.o: $1/%.s
	$(vecho) "AS $$<"
	$(Q) $(AS) $(addprefix -I,$(INCDIR)) $(CFLAGS) -c $$< -o $$@
endif
ifneq (,$(filter $1/%.S,$(SOURCE_FILES)))
$2%.o: $1/%.S
	$(vecho) "AS $$<"
	$(Q) $(AS) $(addprefix -I,$(INCDIR)) $(CFLAGS) -c $$< -o $$@
endif
ifneq (,$(filter $1/%.c,$(SOURCE_FILES)))
$2%.o: $1/%.c $2%.c.d
	$(vecho) "CC $$<"
	$(Q) $(CC) $(addprefix -I,$(INCDIR)) $(CFLAGS) -std=c11 -c $$< -o $$@
$2%.c.d: $1/%.c
	$(Q) $(CC) $(addprefix -I,$(INCDIR)) $(CFLAGS) -std=c11 -MM -MT $2$$*.o $$< -MF $$@
.PRECIOUS: $2%.c.d
endif
ifneq (,$(filter $1/%.cpp,$(SOURCE_FILES)))
$2%.o: $1/%.cpp $2%.cpp.d
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(addprefix -I,$(INCDIR)) $(CXXFLAGS) -c $$< -o $$@
$2%.cpp.d: $1/%.cpp
	$(Q) $(CXX) $(addprefix -I,$(INCDIR)) $(CXXFLAGS) -MM -MT $2$$*.o $$< -MF $$@
.PRECIOUS: $2%.cpp.d
endif
endef

# Resolve a source path to the corresponding build output object file
# $1 -> source root directory
# $2 -> file path(s)
define ResolveObjPath
$(foreach f,$2,$(patsubst $(SMING_HOME)/%,%,$(patsubst $1/%,%,$f)))
endef

# All source files, absolute paths
SOURCE_FILES := $(call AbsoluteSourcePath,$(COMPONENT_PATH),$(COMPONENT_SRCFILES)) \
	$(foreach d,$(call AbsoluteSourcePath,$(COMPONENT_PATH),$(COMPONENT_SRCDIRS)),$(wildcard $d/*.s $d/*.S $d/*.c $d/*.cpp))
# All unique source directories, absolute paths
SOURCE_DIRS := $(sort $(patsubst %/,%,$(dir $(SOURCE_FILES))))
# Output object files
OBJ := $(call ResolveObjPath,$(COMPONENT_PATH),$(SOURCE_FILES))
OBJ := $(OBJ:.s=.o)
OBJ := $(OBJ:.S=.o)
OBJ := $(OBJ:.c=.o)
OBJ := $(OBJ:.cpp=.o)
# Create implicit rules for each source directory, and update BUILD_DIRS
$(foreach d,$(SOURCE_DIRS),$(eval $(call GenerateCompileTargets,$d,$(call ResolveObjPath,$(COMPONENT_PATH),$d/))))
BUILD_DIRS := $(sort $(BUILD_DIRS:/=))
# Include any generated dependency files (these won't exist on first build)
ABS_BUILD_DIRS := $(sort $(COMPONENT_BUILD_DIR) $(BUILD_DIRS))
include $(wildcard $(ABS_BUILD_DIRS:=/*.c.d))
include $(wildcard $(ABS_BUILD_DIRS:=/*.cpp.d))

# Provide a target unless Component is custom built, in which case the component.mk will have defined this already
$(COMPONENT_LIBPATH): $(OBJ) $(EXTRA_OBJ)
	$(vecho) "AR $@"
	$(Q) test ! -f $@ || rm $@
	$(Q) $(AR) rcsP $@ $^

endif # ifeq (,$(CUSTOM_BUILD))
endif # ifneq (,$(COMPONENT_LIBNAME))

TARGET_DIRS := $(sort $(patsubst %/,%,$(dir $(COMPONENT_TARGETS))))

# Targets
.PHONY: build
build: checkdirs $(COMPONENT_TARGETS)

checkdirs: | $(BUILD_DIRS) $(TARGET_DIRS)

$(BUILD_DIRS) $(TARGET_DIRS):
	$(Q) mkdir -p $@
