#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

make -C "$SMING_PROJECTS_DIR/samples/HttpServer_FirmwareUpload" python-requirements PIP_ARGS=--user
$MAKE_PARALLEL samples
make clean samples-clean
$MAKE_PARALLEL Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3
$MAKE_PARALLEL HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1
