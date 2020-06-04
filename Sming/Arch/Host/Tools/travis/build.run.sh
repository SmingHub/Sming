#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

if [[ $CHECK_SCA -eq 1 ]]; then
	$DIR/coverity-scan.sh
else
	$MAKE_PARALLEL Basic_Blink Basic_DateTime Basic_Delegates Basic_Interrupts Basic_ProgMem Basic_Serial Basic_Servo Basic_Ssl LiveDebug DEBUG_VERBOSE_LEVEL=3
fi

# Build and run tests
export SMING_TARGET_OPTIONS='--flashfile=$(FLASH_BIN) --flashsize=$(SPI_SIZE)'
$MAKE_PARALLEL tests

# Build the documentation
mv $SMING_PROJECTS_DIR/samples ..
mv $SMING_PROJECTS_DIR/tests ..
unset SMING_PROJECTS_DIR
make docs V=1
