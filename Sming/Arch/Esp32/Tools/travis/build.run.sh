#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

$MAKE_PARALLEL Basic_Blink Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3
$MAKE_PARALLEL Basic_Ssl ENABLE_SSL=Bearssl DEBUG_VERBOSE_LEVEL=3
