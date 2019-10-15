#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Build times benefit from parallel building
export MAKE_PARALLEL="make -j3"

unset SPIFFY
unset ESPTOOL2
unset SDK_BASE
env

export SMING_HOME=$TRAVIS_BUILD_DIR/Sming

cd $SMING_HOME

# Check coding style
if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
 	make cs
 	DIFFS=$(git diff)
 	if [ "$DIFFS" != "" ]; then
 	  echo "!!! Coding Style issues Found!!!"
 	  echo "    Run: 'make cs' to fix them. "
 	  echo "$DIFFS"
 	  exit 1
 	fi
fi

# Setup ARCH SDK paths
if [ "$SMING_ARCH" == "Esp8266" ]; then
	export ESP_HOME=$TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	export PATH=$PATH:$ESP_HOME/xtensa-lx106-elf/bin:$ESP_HOME/utils/
fi

# Full compile checks please
export STRICT=1

# Move samples and tests into directory outside of the Sming repo.
export SMING_PROJECTS_DIR=$HOME/projects
mkdir $SMING_PROJECTS_DIR
mv ../samples $SMING_PROJECTS_DIR
mv ../tests $SMING_PROJECTS_DIR

# Diagnostic info
cd $SMING_PROJECTS_DIR/samples/Basic_Blink
make help
make list-config

# Check if we could run static code analysis
CHECK_SCA=0
if [[ $TRAVIS_COMMIT_MESSAGE == *"[scan:coverity]"*  && $TRAVIS_PULL_REQUEST != "true" ]]; then
  CHECK_SCA=1
fi


# This will build the Basic_Blink application and most of the framework Components
if [[ $CHECK_SCA -eq 0 ]]; then
  $MAKE_PARALLEL
fi

cd $SMING_HOME

if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
	if [[ $CHECK_SCA -eq 1 ]]; then
		$TRAVIS_BUILD_DIR/.travis/coverity-scan.sh
	else
	  $MAKE_PARALLEL Basic_DateTime Basic_Delegates Basic_Interrupts Basic_ProgMem Basic_Serial Basic_Servo Basic_Ssl LiveDebug DEBUG_VERBOSE_LEVEL=3
	fi

	# Build and run tests
	export SMING_TARGET_OPTIONS='--flashfile=$(FLASH_BIN) --flashsize=$(SPI_SIZE)'
	$MAKE_PARALLEL tests

	# Build the documentation
	mv $SMING_PROJECTS_DIR/samples ..
	mv $SMING_PROJECTS_DIR/tests ..
	unset SMING_PROJECTS_DIR
	make docs V=1
else
	$MAKE_PARALLEL samples
	make clean samples-clean
	$MAKE_PARALLEL Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3
	$MAKE_PARALLEL HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1
fi
