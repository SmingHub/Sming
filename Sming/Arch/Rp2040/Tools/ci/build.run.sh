#!/bin/bash
#
# Rp2040 build.run.sh

RP2040_PROJECTS="Basic_Blink Basic_Serial Basic_Storage"

$MAKE_PARALLEL $RP2040_PROJECTS DEBUG_VERBOSE_LEVEL=3 STRICT=1
