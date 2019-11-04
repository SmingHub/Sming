# Makefile for Sphinx documentation
#

# Don't bother with implicit checks
.SUFFIXES:

SHELL = /bin/bash

SMINGDIR := ..
override SMING_HOME := $(SMINGDIR)/Sming

# You can set these variables from the command line, and also from the environment for the first two.
SPHINXOPTS		?=
SPHINXBUILD		?= sphinx-build
SOURCEDIR		= source
BUILDDIR		= build

# Documentation is copied into this directory so it gets pulled into build
SOURCE_INCDIR	:= $(SOURCEDIR)/_inc

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q			:=
vecho		:= @true
else
Q			:= @
vecho		:= @echo
endif

# Define anything required to get all optional code included
include defines.mk

define Sphinx
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
endef

# Display variable and list values, e.g. $(call PrintVariable,LIBS)
# $1 -> Name of variable containing values
define PrintVariable
	$(info $1)
	$(foreach item,$($1),$(info - $(item)))
endef

# $1 -> Directories to look for README and image files
define FindSourceFiles
$(foreach d,$1,$(wildcard $d/*.rst $d/README.md $d/*.svg $d/*.png $d/*.jpg))
endef

# Translate a source path into the corresponding INCDIR location
# $1 -> Path(s) to source file in code tree
define GetIncPath
$(patsubst $(SMINGDIR)/%,$(SOURCE_INCDIR)/%,$1)
endef

# Translate a source path into the corresponding document link path
# $1 -> Path(s) to source file in code tree
define GetDocPath
$(patsubst $(SMINGDIR)/%,/_inc/%,$1)
endef

# Put it first so that "make" without argument is like "make help".
.PHONY: help
help:
	$(call Sphinx)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
.PHONY: html singlehtml latex
html singlehtml latex:
	$(call Sphinx)

.PHONY: latexpdf
latexpdf: latex
	@echo "Running LaTeX files through pdflatex..."
	$(Q) $(MAKE) -C $(BUILDDIR)/latex all-pdf

# List immediate sub-directories for a list of root directories
# Results are sorted and without trailing path separator
# $1 -> Root paths
define ListSubDirs
$(sort $(foreach d,$(dir $(wildcard $1/*/.)),$(d:/=)))
endef

# Non-component locations to copy docs from
SOURCE_DIRS := \
	$(SMINGDIR)/docs \
	$(call ListSubDirs,$(SMING_HOME)/Arch)

# List of documentation files to be included from source tree
SOURCE_FILES := \
	$(SMINGDIR)/CONTRIBUTING.md \
	$(call FindSourceFiles,$(SOURCE_DIRS))

# Each sample, library and Component MUST have a README.rst or README.md file
# This is a list of directories where such files may exist
# If a component.mk file exists, then it is parsed and any dependencies handled
COMPONENT_ROOT_DIRS := \
	$(wildcard $(SMING_HOME)/Arch/*/Components) \
	$(SMING_HOME)/Components \
	$(SMING_HOME)/Libraries \
	$(SMINGDIR)/samples

# All directories where a README file should/must exist, and where a component.mk file could exist
COMPONENT_DIRS := $(SMING_HOME) $(foreach d,$(COMPONENT_ROOT_DIRS),$(call ListSubDirs,$d))

# Fetch a list of URLs once rather than invoking shell each time we need a lookup
SUBMODULE_URLS := $(shell awk '/path = .*/ { path = $$3 } /url = .*/ { printf "%s=%s ", path, $$3 } ' $(SMINGDIR)/.gitmodules)

# Lookup the URL for a submodule given its path
# $1 -> path
define GetSubmoduleURL
$(subst $1=,,$(filter $1=%,$(SUBMODULE_URLS)))
endef

# If a Sphinx 'include' directive starts with '/' that's the source directory, so we need a path relative to that
GetIncludePath = /../$(SMINGDIR)/$(patsubst $(abspath $(SMINGDIR))/%,%,$(abspath $1))

# For Windows, Sphinx has a problem with the /c/ style drives and wants c:/ instead
ifeq ($(OS),Windows_NT)
DRIVE				:= $(firstword $(subst /, ,$(CURDIR)))
GetMdIncludePath	= $(DRIVE):/$(subst /$(DRIVE)/,,$(abspath $1))
else
GetMdIncludePath	= $(abspath $1)
endif

# $1 -> Path to a Component, sample or Library
define GetComponentType
$(if $(findstring /Libraries/,$1),Library,$(if $(findstring /samples/,$1),Sample,Component))
endef

include index.mk

# Used to escape generated text so it can be handled as a single line
define NewLine


endef

# This macro sets the default component variables before including the (optional) component.mk file.
# $1 -> name
# $2 -> path
define ParseComponent
$(if $V,$(info -- Parsing $2))
COMPONENT_SUBMODULES	:=
COMPONENT_DOCFILES		:=
# Process any component.mk file (optional)
COMPONENT_RULE			:= __no_build__
COMPONENT_NAME			:= $1
COMPONENT_PATH			:= $2
COMPONENT_VARS			:=
CONFIG_VARS				:=
RELINK_VARS				:=
CACHE_VARS				:=
DEBUG_VARS				:=
COMPONENT_DEPENDS		:=
ARDUINO_LIBRARIES		:=
include defines.mk
-include $2/component.mk
CMP_$2_VARS				:= $$(sort $$(COMPONENT_VARS))
CMP_$2_LIBRARIES		:= $$(ARDUINO_LIBRARIES)
CMP_$2_SUBMODULES		:= $$(COMPONENT_SUBMODULES)
CMP_$2_DIRS := \
	$$(COMPONENT_PATH) \
	$$(addprefix $$(COMPONENT_PATH)/,$$(COMPONENT_SUBMODULES))
CMP_$2_FILES := \
	$$(call FindSourceFiles,$$(CMP_$2_DIRS)) \
	$$(wildcard $$(addprefix $$(COMPONENT_PATH)/,$$(COMPONENT_DOCFILES)))
CMP_$2_README			:= $$(filter $2/README.%,$$(CMP_$2_FILES))
CMP_$2_FILES			:= $$(filter-out $$(CMP_$2_README),$$(CMP_$2_FILES))
SOURCE_FILES			+= $$(CMP_$2_FILES)

#
COMPONENT_ENVVARS		:= $$(sort $$(COMPONENT_VARS) $$(CONFIG_VARS) $$(RELINK_VARS) $$(CACHE_VARS) $$(DEBUG_VARS))
CMP_$2_ENVVARS			:= $$(COMPONENT_ENVVARS)

CMP_$2_DEPENDS	:= $$(COMPONENT_DEPENDS)
$$(foreach c,$$(COMPONENT_DEPENDS) $$(ARDUINO_LIBRARIES),$$(eval CMP_$$c_XREF += $2))

# Resolve dependencies
#
# If this is an Arch component, then match this Arch only, or **main components** (e.g. Host/heap -> malloc_count)
#
ifneq (,$(findstring /Arch/,$2))
FILTERED_DIRS := $$(filter $(dir $2)% $(SMING_HOME)/Components/%,$(COMPONENT_DIRS))
else
FILTERED_DIRS := $(COMPONENT_DIRS)
endif
CMP_$2_DEPEND_DIRS := \
	$(if $(findstring /samples/,$2),/_inc/Sming) \
	$$(foreach d,$$(call GetDocPath,$$(foreach c,$$(COMPONENT_DEPENDS),$$(filter %/$$c,$$(FILTERED_DIRS)))),$$d)

CMP_$2_INDEX = $$(subst $$(NewLine),\n,$$(call GenIndex,$1,$2,$(patsubst $(SMINGDIR)/%,%,$2)))

$$(call GetIncPath,$$(COMPONENT_PATH))/index.rst: $$(CMP_$2_README) $$(call GetIncPath,$$(CMP_$2_FILES))
ifeq ($V,1)
	$$(info Creating $$@...)
endif
	@echo -e '$$(CMP_$2_INDEX)' > $$@

endef # ParseComponent

$(foreach d,$(COMPONENT_DIRS),$(eval $(call ParseComponent,$(notdir $d),$d)))

SOURCE_FILE_DIRS		:= $(sort $(dir $(SOURCE_FILES)))
MISSING_README_DIRS		:= $(filter-out $(SOURCE_FILE_DIRS:/=),$(SOURCE_DIRS))

.PHONY: clean
clean:
	$(Q) rm -rf $(SOURCE_INCDIR) api $(BUILDDIR)

SOURCE_INCDIRS			:= $(sort $(call GetIncPath,$(COMPONENT_DIRS) $(SOURCE_FILE_DIRS)))
# Each Sample, Library or Component README.* is included directly rather than added to the toctree
SOURCE_FILES			:= $(filter-out $(addsuffix /README.%,$(COMPONENT_DIRS)),$(SOURCE_FILES))
SOURCE_INCFILES			:= $(call GetIncPath,$(SOURCE_FILES))
COMPONENT_INDEXFILES	:= $(addsuffix /index.rst,$(call GetIncPath,$(COMPONENT_DIRS)))

.PHONY: setup
setup: checkdirs $(SOURCE_INCFILES) $(COMPONENT_INDEXFILES)
ifeq ($V,1)
	$(call PrintVariable,SOURCE_DIRS)
	$(call PrintVariable,SOURCE_FILES)
	$(call PrintVariable,COMPONENT_INCFILES)
endif
	$(if $(MISSING_README_DIRS),$(call PrintVariable,MISSING_README_DIRS))

# Copy source -> dest file
# $1 -> Source file
# $2 -> Destination file
define CopyFileTarget
$1: $2
	$(Q) cp $$< $$@
endef

$(foreach f,$(SOURCE_FILES),$(eval $(call CopyFileTarget,$(call GetIncPath,$f),$f)))


.PHONY: checkdirs
checkdirs:
	$(Q) mkdir -p $(SOURCE_INCDIRS)

.PHONY: api
api: api/xml/index.xml

#
DOXYGEN := $(shell command -v doxygen 2> /dev/null)

api/error.log:
ifndef DOXYGEN
	$(error doxygen not found - not building API docs)
endif
	@echo Generating Doxygen information
	$(Q) mkdir -p api api/html
	$(Q) doxygen 2>$@ 1>$(@D)/doxygen.log
	-$(Q) rm api/html/api/*.md5 api/html/api/*.map

api/xml/index.xml: api/error.log
	@echo "Undocumented:"
	@echo "- Compounds:  $(shell grep "Compound.*is not documented" $< | wc -l)"
	@echo "- Members:    $(shell grep "Member.*is not documented" $< | wc -l) "
	@echo "- Parameters: $(shell grep "The following parameters of .* are not documented" $< | wc -l) "
	@echo "- Symbols:    $(shell grep "documented symbol.*not declared" $< | wc -l)"
	@echo "Unknown:"
	@echo " - Commands:  $(shell grep "Found unknown command" $< | wc -l)"
	@echo "Not Found:"
	@echo " - Arguments: $(shell grep "argument.*not found" $< | wc -l)"
	@echo " - Files:     $(shell grep "included file.*not found" $< | wc -l)"
	@echo " - Links:     $(shell grep "link request.*could not be resolved" $< | wc -l)"
