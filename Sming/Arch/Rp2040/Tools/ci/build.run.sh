#!/bin/bash
#
# Rp2040 build.run.sh

$MAKE_PARALLEL samples DEBUG_VERBOSE_LEVEL=3 STRICT=1

$MAKE_PARALLEL component-samples STRICT=1
