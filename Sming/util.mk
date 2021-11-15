#
# Make utility definitions
#
# These definitions must have no external dependencies.
# They are used for both code and documentation builds.
#

# Convert Windows paths to POSIX paths
ifeq ($(OS),Windows_NT)
# Powershell does weird things to this variable, revert to default
override MAKE	:= make
FixPath			= $(subst //,/,$(subst \,/,$(addprefix /,$(subst :,,$1))))
else
FixPath			= $1
endif

V ?= $(VERBOSE)
ifeq ("$(V)","1")
	Q			:=
	vecho		:= @true
else
	Q			:= @
	vecho		:= @echo
endif

# Convert string to upper/lower case
# 1 -> String
ToUpper = $(shell echo "$1" | tr 'a-z' 'A-Z')
ToLower = $(shell echo "$1" | tr 'A-Z' 'a-z')

# Calculate a hash string for appending to library names, etc.
# $1 -> Name of variable containing data to be hashed
define CalculateVariantHash
$(firstword $(shell echo -n $($1) | md5sum -t))
endef

# Fetch full path for submodules matching given pattern
# Note that scanning .gitmodules is considerably quicker than using GIT
# $1 -> Path to repo working directory
# $2 -> Path pattern to match
define ScanGitModules
$(patsubst %,$(abspath $1/%),$(subst path = ,,$(shell grep -o 'path = $2' '$1/.gitmodules')))
endef

# List immediate sub-directories for a list of root directories
# Results are sorted and without trailing path separator
# $1 -> Root paths
define ListSubDirs
$(foreach d,$(dir $(wildcard $1/*/.)),$(d:/=))
endef

# Check that $2 is a valid sub-directory of $1. Return empty string if not.
# $1 -> Parent directory
# $2 -> Sub-directory
# During wildcard searches, paths with spaces cause recursion.
define IsSubDir
$(if $(subst $(1:/=),,$(2:/=)),$(findstring $(1:/=),$2),)
endef

# List sub-directories recursively for a single root directory
# Results are sorted and without trailing path separator
# Sub-directories with spaces are skipped
# $1 -> Root path
define ListAllSubDirsSingle
$(foreach d,$(dir $(wildcard $1/*/.)),$(if $(call IsSubDir,$1,$d),$(d:/=) $(call ListAllSubDirs,$(d:/=))))
endef

# List sub-directories recursively for a list of root directories
# Results are sorted and without trailing path separator
# Sub-directories with spaces are skipped
# $1 -> Root paths
define ListAllSubDirs
$(foreach d,$1,$(call ListAllSubDirsSingle,$d))
endef

# Recursively search list of directories for matching files
# $1 -> Directories to scan
# $2 -> Filename filter
define ListAllFiles
$(wildcard $(foreach d,$(call ListAllSubDirs,$1),$d/$2))
endef


# Display variable and list values, e.g. $(call PrintVariable,LIBS)
# $1 -> Name of variable containing values
# $2 -> (optional) tag to use instead of variable name
define PrintVariable
	$(info $(if $2,$2,$1):)
	$(foreach item,$($1),$(info - $(item)))
endef

define PrintVariableSorted
	$(info $(if $2,$2,$1):)
	$(foreach item,$(sort $($1)),$(info - $(value item)))
endef

# Display list of variable references with their values e.g. $(call PrintVariableRefs,DEBUG_VARS)
# $1 -> Name of variable containing list of variable names
# $2 -> (optional) tag to use instead of variable name
define PrintVariableRefs
	$(info $(if $2,$2,$1):)
	$(foreach item,$(sort $($1)),$(info - $(item) = $(value $(item))) )
endef

#
# Get directory without trailing separator
# $1 -> List of directories
dirx = $(patsubst %/,%,$(dir $1))

# Give relative or absolute source paths, convert them all to absolute
# $1 -> source root directory
# $2 -> file path(s)
define AbsoluteSourcePath
$(foreach f,$2,$(abspath $(if $(filter /%,$f),$f,$1/$f)))
endef

# Run a command in a new terminal window
# $1 -> Command to execute
ifeq ($(UNAME),Windows)
DetachCommand = start $1
else
DetachCommand = gnome-terminal -- bash -c "sleep 1; $1"
endif


# List of all soc configuration files
export SOC_CONFIG_FILES = $(sort $(wildcard $(SMING_HOME)/Arch/*/*-soc.json))
AVAILABLE_SOCS := $(patsubst %-soc.json,%,$(notdir $(SOC_CONFIG_FILES)))

define SocFromPath
$(patsubst %-soc.json,%,$(notdir $1))
endef

# Provide variable for each architecture listing available SOCs
# $1 -> Architecture name
define SetArchSocs
ARCH_$1_SOC := $(sort $(call SocFromPath,$(wildcard $(SMING_HOME)/Arch/$1/*-soc.json)))
endef
$(foreach a,$(call ListSubDirs,$(SMING_HOME)/Arch),$(eval $(call SetArchSocs,$(notdir $a))))
