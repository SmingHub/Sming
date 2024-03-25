#!/bin/bash
#
# Host build.run.sh

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

if [[ $CHECK_SCA -eq 1 ]]; then
	"$DIR/coverity-scan.sh"
else
	$MAKE_PARALLEL samples DEBUG_VERBOSE_LEVEL=3
	$MAKE_PARALLEL component-samples DEBUG_VERBOSE_LEVEL=3
fi

# Build and run tests
export SMING_TARGET_OPTIONS='--flashfile=$(FLASH_BIN) --flashsize=$(SPI_SIZE)'
$MAKE_PARALLEL tests
