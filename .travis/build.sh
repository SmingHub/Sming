#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Build times benefit from parallel building
export MAKE_PARALLEL="make -j3"

export SMING_HOME=$TRAVIS_BUILD_DIR/Sming

# Setup ARCH SDK
cd $SMING_HOME
if [ -f "$SMING_HOME/Arch/$SMING_ARCH/Tools/travis/build.setup.sh" ]; then
   source "$SMING_HOME/Arch/$SMING_ARCH/Tools/travis/build.setup.sh"
fi

env

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

# Run ARCH SDK tests
cd $SMING_HOME
if [ -f "$SMING_HOME/Arch/$SMING_ARCH/Tools/travis/build.run.sh" ]; then
   source "$SMING_HOME/Arch/$SMING_ARCH/Tools/travis/build.run.sh"
fi
