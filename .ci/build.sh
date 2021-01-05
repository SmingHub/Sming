#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Build times benefit from parallel building
export MAKE_PARALLEL="make -j3"

# Setup ARCH SDK
setup() {
	cd $SMING_HOME
	source Arch/$SMING_ARCH/Tools/ci/build.setup.sh
}

export SMING_ARCH=Host
setup
export SMING_ARCH=Esp8266
setup
export SMING_ARCH=Esp32
setup

env

# Move samples and tests into directory outside of the Sming repo.
export SMING_PROJECTS_DIR=$HOME/projects
mkdir -p $SMING_PROJECTS_DIR
mv samples $SMING_PROJECTS_DIR
mv tests $SMING_PROJECTS_DIR

# Full compile checks please
export STRICT=1

run_tests() {
	# Diagnostic info
	cd $SMING_PROJECTS_DIR/samples/Basic_Blink
	make help
	make list-config
	
	# Run ARCH SDK tests
	cd $SMING_HOME
	source Arch/$SMING_ARCH/Tools/ci/build.run.sh
}

export SMING_ARCH=Host
run_tests

export SMING_ARCH=Esp8266
run_tests

export SMING_ARCH=Esp32
run_tests
