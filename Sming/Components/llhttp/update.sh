#!/usr/bin/env bash
#
# Update and patch llhttp source code from repository
#

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 [git path]"
    echo "   - Clone https://github.com/nodejs/llhttp.git to working directory"
    echo "   - Build as per instructions"
    echo "   - Run this script to update Sming source"
    exit 1
fi

srcdir="$1"

srcfiles=(\
    build/c/llhttp.c \
    build/llhttp.h \
    src/native/api.c \
    src/native/http.c \
)

for f in ${srcfiles[@]}; do
    cp "$srcdir/$f" src/
done

# Using SED on generated code as changes are more likely

SED="sed -i -E"

$SED 's/(state->reason = )"(.+)"/\1PSTR("\2")/' src/llhttp.c
$SED 's/static (uint8_t lookup_table\[\]) =/static const \1 PROGMEM =/' src/llhttp.c
$SED 's/(lookup_table\[\(uint8_t\) \*p\])/pgm_read_byte\(\&\1)/' src/llhttp.c

$SED '/^#include <stdint.h>$/a#include <FakePgmSpace.h>' src/llhttp.h
$SED '/^enum llhttp_method \{$/,/^};$/s/HTTP_/HTTP_METHOD_/' src/llhttp.h
$SED 's/HTTP_CONNECT/HTTP_METHOD_CONNECT/' src/http.c

$SED 's/("Span callback error in " #NAME)/PSTR(\1)/' src/api.c

# Remove unused functions
$SED '/llhttp_errno_name/,/^}/d' src/api.c
$SED '/llhttp_method_name/,/^}/d' src/api.c
$SED '/llhttp_status_name/,/^}/d' src/api.c
