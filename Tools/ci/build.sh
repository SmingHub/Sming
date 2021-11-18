#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Build times benefit from parallel building
export MAKE_PARALLEL="make -j$(nproc)"

cd "$SMING_HOME"
source "Arch/$SMING_ARCH/Tools/ci/build.setup.sh"

# Don't leak this!
unset SMING_SECRET

env

# Move samples and tests into directory outside of the Sming repo.
export SMING_PROJECTS_DIR=$HOME/projects
mkdir -p "$SMING_PROJECTS_DIR"
cd "$SMING_HOME/.."
mv samples "$SMING_PROJECTS_DIR"
mv tests "$SMING_PROJECTS_DIR"

# Full compile checks please
export STRICT=1

# Diagnostic info
cd "$SMING_PROJECTS_DIR/samples/Basic_Blink"
make help
make list-config

$MAKE_PARALLEL

# HostTests should build and run on all architectures
$MAKE_PARALLEL -C "$SMING_PROJECTS_DIR/tests/HostTests"

# Run ARCH build/tests
cd "$SMING_HOME"
source "Arch/$SMING_ARCH/Tools/ci/build.run.sh"
