#
# This example demonstrates use of a shared Component in a repository (directory) outside of Sming or the
# project which uses it.
#
# Placing Components in a common location allows them to be used by multiple projects. Just add the
# name to `COMPONENT_DEPENDS` in your project's component.mk file.
#
# Here's our example:
#
#	|_ Project				The example Sming project
#	|_ print-test.repo		The GIT repository containing the source for our submodule
#							This is where Sming will pull the submodule from when our Project is built
#	|_ shared/Components	The Component repository
#		|_ shared-test		The example Component
#			|_ print-test	The submodule
#
# Note that Components don't need to be in a GIT repo, just set up a directory somewhere and add
# its full path to COMPONENT_SEARCH_DIRS.
#
# However, if the Component uses submodules then of course it needs to be in a GIT repo. This makefile
# does all that before building the example project.
#

ifeq ($(V),1)
Q :=
else
Q := @
endif

REPO_DIR				:= print-test.repo
SUBMODULE_NAME			:= print-test
COMPONENT_NAME			:= shared-test
SHARED_COMPONENT_DIR	:= shared/Components

# Tell Sming where our repository is - absolute path required
export COMPONENT_SEARCH_DIRS := $(CURDIR)/$(SHARED_COMPONENT_DIR)

.PHONY: execute
execute: setup run

.PHONY: run
run:
	$(MAKE) -C Project run

.NOTPARALLEL:
.PHONY: setup
setup: | $(REPO_DIR)/.git $(SHARED_COMPONENT_DIR)/.gitmodules

clean:
	$(Q) $(MAKE) -C Project clean
	$(Q) rm -rf $(REPO_DIR)/.git $(SHARED_COMPONENT_DIR)/.git $(SHARED_COMPONENT_DIR)/.gitmodules $(SHARED_COMPONENT_DIR)/$(COMPONENT_NAME)/$(SUBMODULE_NAME)

# Create GIT repository and commit files to it
$(REPO_DIR)/.git:
	$(Q) cd $(@D) && \
	git init && \
	git add . && \
	git config --add user.email 'test@sming.dev' && \
	git config --add user.name 'test' && \
	git commit -m 'Initial Commit'


# Add shared-test.repo as a submodule to the shared-test Component, but leave it un-initialised
$(SHARED_COMPONENT_DIR)/.gitmodules: | $(SHARED_COMPONENT_DIR)/.git
	$(Q) cd $(@D)/$(COMPONENT_NAME) && \
	git submodule add $(CURDIR)/$(REPO_DIR) $(SUBMODULE_NAME) && \
	git submodule deinit $(SUBMODULE_NAME) -f

$(SHARED_COMPONENT_DIR)/.git:
	$(Q) git init $(@D)
