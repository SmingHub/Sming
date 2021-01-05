#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Build times benefit from parallel building
export MAKE_PARALLEL="make -j3"

# Setup ARCH SDK
setup() {
	export SMING_ARCH=$1
	cd $SMING_HOME
	source Arch/$1/Tools/ci/build.setup.sh
}

setup Host
setup Esp8266
setup Esp32

env

# Move samples and tests into directory outside of the Sming repo.
export SMING_PROJECTS_DIR=$HOME/projects
mkdir -p $SMING_PROJECTS_DIR
cd $SMING_HOME/..
mv samples $SMING_PROJECTS_DIR
mv tests $SMING_PROJECTS_DIR

# Full compile checks please
export STRICT=1

run() {
	export SMING_ARCH=$1

	# Diagnostic info
	cd $SMING_PROJECTS_DIR/samples/Basic_Blink
	make help
	make list-config
	
	# Run ARCH SDK tests
	cd $SMING_HOME
	source Arch/$1/Tools/ci/build.run.sh
}

run Host
run Esp8266
run Esp32
